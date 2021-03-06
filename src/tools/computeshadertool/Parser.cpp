/**
 * @file
 */

#include "Parser.h"
#include "core/Log.h"
#include "core/Assert.h"
#include "core/String.h"
#include "Types.h"
#include "Util.h"
#include <simplecpp.h>
#include <stack>

namespace computeshadertool {

static bool validate(Kernel& kernel) {
	bool error = false;
	// check mutual exclusive parameter flags
	for (const Parameter& p : kernel.parameters) {
		if ((p.flags & compute::BufferFlag::UseHostPointer) != compute::BufferFlag::None) {
			if ((p.flags & compute::BufferFlag::CopyHostPointer) != compute::BufferFlag::None) {
				Log::error("CopyHostPointer and UseHostPointer are mutually exclusive");
				error = true;
			}
			if ((p.flags & compute::BufferFlag::AllocHostPointer) != compute::BufferFlag::None) {
				Log::error("AllocHostPointer and UseHostPointer are mutually exclusive");
				error = true;
			}
		}
		if ((p.flags & compute::BufferFlag::ReadWrite) != compute::BufferFlag::None) {
			if ((p.flags & compute::BufferFlag::ReadOnly) != compute::BufferFlag::None) {
				Log::error("ReadWrite and ReadOnly are mutually exclusive");
				error = true;
			}
			if ((p.flags & compute::BufferFlag::WriteOnly) != compute::BufferFlag::None) {
				Log::error("ReadWrite and WriteOnly are mutually exclusive");
				error = true;
			}
		}
		if ((p.flags & compute::BufferFlag::ReadOnly) != compute::BufferFlag::None
		 && (p.flags & compute::BufferFlag::WriteOnly) != compute::BufferFlag::None) {
			if ((p.flags & compute::BufferFlag::WriteOnly) != compute::BufferFlag::None) {
				Log::error("ReadOnly and WriteOnly are mutually exclusive");
				error = true;
			}
		}
	}
	return !error;
}

static const simplecpp::Token *parseStruct(const simplecpp::Token *tok, std::vector<Struct>& structs) {
	tok = tok->next;
	if (!tok) {
		Log::error("%s:%i:%i: error: Failed to parse struct - not enough tokens - expected name",
				tok->location.file().c_str(), tok->location.line, tok->location.col);
		return tok;
	}
	Struct structVar;
	structVar.name = tok->str;
	tok = tok->next;
	if (!tok->next) {
		Log::error("%s:%i:%i: error: Failed to parse struct - not enough tokens",
				tok->location.file().c_str(), tok->location.line, tok->location.col);
		return tok;
	}
	if (tok->str != "{") {
		for (; tok; tok = tok->next) {
			const std::string& token = tok->str;
			if (token == ";") {
				structs.push_back(structVar);
				tok = tok->next;
				break;
			} else {
				Log::error("%s:%i:%i: error: Failed to parse struct - invalid token: %s",
						tok->location.file().c_str(), tok->location.line, tok->location.col, token.c_str());
				return tok;
			}
		}
	}
	if (!tok) {
		return nullptr;
	}
	int depth = 1;
	bool valid = false;
	Parameter param;
	for (tok = tok->next; tok; tok = tok->next) {
		const std::string& token = tok->str;
		if (token == "{") {
			++depth;
		} else if (token == "}") {
			--depth;
			if (depth <= 0) {
				valid = true;
				break;
			}
		} else if (tok->next && (tok->next->str == ";" || tok->next->str == "[")) {
			param.name = token;
			if (tok->next->str == "[") {
				for (tok = tok->next; tok; tok = tok->next) {
					param.name.append(tok->str);
					if (tok->str == "]") {
						break;
					}
				}
			}
			structVar.parameters.push_back(param);
			param = Parameter();
			if (tok) {
				tok = tok->next;
			}
		} else {
			if (util::isQualifier(token)) {
				param.qualifier = token;
			} else {
				if (param.type.empty()) {
					param.type = token;
				} else {
					param.type.append(" ");
					param.type.append(token);
				}
			}
		}
	}
	if (valid) {
		structs.push_back(structVar);
	}
	return tok;
}

static const simplecpp::Token *parseKernel(const simplecpp::Token *tok, std::vector<Kernel>& kernels) {
	if (!tok) {
		return tok;
	}
	std::stack<std::string> stack;
	std::string prev;
	int inAttribute = 0;
	for (tok = tok->next; tok; tok = tok->next) {
		std::string token = tok->str;
		if (token == "{") {
			break;
		}
		if (core::string::startsWith(token, "__attribute__")) {
			if (!tok->next) {
				continue;
			}
			tok = tok->next;
			if (tok->str != "(") {
				tok = tok->previous;
				continue;
			}
			++inAttribute;
			for (tok = tok->next; tok; tok = tok->next) {
				token = tok->str;
				if (token == "(") {
					++inAttribute;
				} else if (token == ")") {
					--inAttribute;
					if (inAttribute == 0) {
						break;
					}
				}
			}
		}

		stack.push(token);
		if (!tok) {
			break;
		}
	}

	if (stack.empty()) {
		Log::error("Could not identify any kernel");
		return tok;
	}

	int kernelDimensions = 1;
	core_assert(tok->str == "{");
	for (tok = tok->next; tok; tok = tok->next) {
		const std::string& token = tok->str;
		if (token == "}") {
			break;
		}
		if (token == "get_global_id" || token == "get_global_size") {
			Log::debug("found %s", token.c_str());
			tok = tok->next;
			if (!tok) {
				Log::error("Expected (");
				return tok;
			}
			if (tok->str != "(") {
				Log::error("Expected ( - got %s", tok->str.c_str());
				return tok;
			}
			tok = tok->next;
			if (!tok) {
				Log::error("Expected number");
				return tok;
			}
			if (!tok->number) {
				Log::error("Expected number, got %s", tok->str.c_str());
				return tok;
			}
			const int dimension = (int)(tok->str[0] - '0') + 1;
			Log::debug("found dimension %i", dimension);
			kernelDimensions = std::max(kernelDimensions, dimension);
			tok = tok->next;
			if (!tok) {
				Log::error("Expected )");
				return tok;
			}
			if (tok->str != ")") {
				Log::error("Expected ) - got %s", tok->str.c_str());
				return tok;
			}
		}
	}

	std::vector<std::string> parameterTokens;
	while (!stack.empty()) {
		const std::string token = stack.top();
		stack.pop();
		if (token == ")") {
			continue;
		}
		if (token == "(") {
			break;
		}
		parameterTokens.insert(parameterTokens.begin(), token);
	}

	if (stack.empty()) {
		Log::error("Expected to get a method name");
		return tok;
	}

	Kernel kernel;
	kernel.name = stack.top();
	kernel.workDimension = kernelDimensions;
	Log::debug("found kernel %s with dimension %i", kernel.name.c_str(), kernel.workDimension);
	stack.pop();

	bool added = false;
	Parameter parameter;
	while (!parameterTokens.empty()) {
		const std::string token = parameterTokens.back();
		parameterTokens.pop_back();
		if (token.empty()) {
			continue;
		}
		if (token == ",") {
			core_assert(!parameter.name.empty());
			if (core::string::startsWith(parameter.name, "*")) {
				parameter.name = parameter.name.substr(1, parameter.name.size());
				parameter.type.append(" *");
			}
			kernel.parameters.insert(kernel.parameters.begin(), parameter);
			parameter = Parameter();
			continue;
		}
		// TODO: __local size must be the size in bytes for the buffer that create
		// TODO: handle these: __global, __local, __private
		if (core::string::startsWith(token, "__")) {
			// The "__" prefix is not required before the qualifiers, but we will continue to use the
			// prefix in this text for consistency. If the qualifier is not specified, the variable
			// gets allocated to "__private", which is the default qualifier.
			if (core::string::startsWith(&token[2], "constant")
			 || core::string::startsWith(&token[2], "read_only")) {
				parameter.flags &= ~(compute::BufferFlag::ReadWrite);
				parameter.flags |= compute::BufferFlag::ReadOnly;
			} else if (core::string::startsWith(&token[2], "write_only")) {
				parameter.flags &= ~(compute::BufferFlag::ReadWrite);
				parameter.flags |= compute::BufferFlag::WriteOnly;
				parameter.byReference = true;
			}
			continue;
		} else {
			if (token == "constant" || token == "read_only") {
				parameter.flags &= ~(compute::BufferFlag::ReadWrite);
				parameter.flags |= compute::BufferFlag::ReadOnly;
				parameter.qualifier = "const";
				continue;
			} else if (token == "write_only") {
				parameter.flags &= ~(compute::BufferFlag::ReadWrite);
				parameter.flags |= compute::BufferFlag::WriteOnly;
				parameter.byReference = true;
				continue;
			}
		}

		added = true;
		if (parameter.name.empty()) {
			parameter.name = token;
		} else if (token == "const") {
			core_assert_msg(parameter.qualifier.empty(), "found %s, but already have %s",
					token.c_str(), parameter.qualifier.c_str());
			parameter.flags &= ~(compute::BufferFlag::ReadWrite | compute::BufferFlag::WriteOnly);
			parameter.flags |= compute::BufferFlag::ReadOnly;
			parameter.qualifier = token;
		} else {
			// TODO: opencl data types - image2d_t, sampler_t
			if (!parameter.type.empty()) {
				parameter.type = token + " " + parameter.type;
			} else {
				parameter.type = token;
			}
		}
	}
	if (added) {
		if (core::string::startsWith(parameter.name, "*")) {
			parameter.name = parameter.name.substr(1, parameter.name.size());
			parameter.type.append(" *");
		}
		core_assert(!parameter.name.empty());
		kernel.parameters.insert(kernel.parameters.begin(), parameter);
	}

	std::stack<std::string> returnTokens;
	while (!stack.empty()) {
		const std::string token = stack.top();
		stack.pop();
		returnTokens.push(token);
	}

	if (returnTokens.empty()) {
		Log::error("Could not find return values");
		return tok;
	}

	while (!returnTokens.empty()) {
		const std::string token = returnTokens.top();
		returnTokens.pop();
		if (!kernel.returnValue.type.empty()) {
			kernel.returnValue.type.append(" ");
		}
		kernel.returnValue.type.append(token);
	}

	if (!validate(kernel)) {
		return tok;
	}

	kernels.push_back(kernel);

	return tok;
}

bool parse(const std::string& buffer, const std::string& computeFilename, std::vector<Kernel>& kernels,
		std::vector<Struct>& structs, std::map<std::string, std::string>& constants) {
	simplecpp::DUI dui;
	simplecpp::OutputList outputList;
	std::vector<std::string> files;
	std::stringstream f(buffer);
	simplecpp::TokenList rawtokens(f, files, computeFilename, &outputList);
	std::map<std::string, simplecpp::TokenList*> included = simplecpp::load(rawtokens, files, dui, &outputList);
	simplecpp::TokenList output(files);
	simplecpp::preprocess(output, rawtokens, files, included, dui, &outputList);

	simplecpp::Location loc(files);
	std::stringstream comment;
	for (const simplecpp::Token *tok = output.cfront(); tok != nullptr; tok = tok->next) {
		const std::string& token = tok->str;
		if (tok->comment) {
			comment << token;
			continue;
		}
		if (token == "__kernel" || token == "kernel") {
			tok = parseKernel(tok, kernels);
		} else if (token == "struct") {
			tok = parseStruct(tok, structs);
		} else if (token == "$constant") {
			if (!tok->next) {
				return false;
			}
			tok = tok->next;
			const std::string varname = tok->str;
			if (!tok->next) {
				return false;
			}
			tok = tok->next;
			const std::string varvalue = tok->str;
			if (!constants.insert(std::make_pair(varname, varvalue)).second) {
				Log::error("Could not register constant %s with value %s (duplicate)", varname.c_str(), varvalue.c_str());
				return false;
			}
		}
		comment.clear();
		if (tok == nullptr) {
			break;
		}
	}
	Log::info("Found %i kernels", (int)kernels.size());
	simplecpp::cleanup(included);
	return true;
}

}
