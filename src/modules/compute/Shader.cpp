/**
 * @file
 */
#include "Shader.h"
#include "core/Var.h"
#include "core/Log.h"
#include "core/App.h"
#include "io/Filesystem.h"
#include "util/IncludeUtil.h"

namespace compute {

Shader::~Shader() {
	shutdown();
}

bool Shader::init() {
	if (!compute::supported()) {
		return false;
	}
	if (_initialized) {
		return true;
	}
	_initialized = true;
	return _initialized;
}

std::string Shader::handlePragmas(const std::string& buffer) const {
	// TODO: check the code for printf statements and activate the pragmas
	//#pragma OPENCL EXTENSION cl_amd_printf : enable
	//#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
	return buffer;
}

void Shader::update(uint32_t deltaTime) {
	core_assert(_initialized);
}

bool Shader::activate() const {
	core_assert(_initialized);
	_active = true;
	return _active;
}

bool Shader::deactivate() const {
	if (!_active) {
		return false;
	}

	_active = false;
	return _active;
}

void Shader::shutdown() {
	_initialized = false;
	_active = false;
	compute::deleteProgram(_program);
}

bool Shader::load(const std::string& name, const std::string& buffer) {
	core_assert(_initialized);
	_name = name;
	const std::string& source = getSource(buffer);
	_program = compute::createProgram(source);
	if (_program == InvalidId) {
		return false;
	}
	return compute::configureProgram(_program);
}

// see https://software.intel.com/sites/default/files/managed/f1/25/opencl-zero-copy-in-opencl-1-2.pdf
BufferFlag Shader::bufferFlags(const void* bufPtr, size_t size) const {
	if ((uintptr_t) bufPtr % compute::requiredAlignment() != 0) {
		return BufferFlag::None;
	}
	if (size % 64 != 0) {
		return BufferFlag::None;
	}
	return BufferFlag::UseHostPointer;
}

void* Shader::bufferAlloc(size_t &size) const {
	const size_t alignment = compute::requiredAlignment();

	// round up to 64 bytes - according to intel opencl zero copy hints
	size = size + (~size + 1) % 64;

	core_assert(size >= sizeof(void*));
	core_assert(size / sizeof(void*) * sizeof(void*) == size);

	// allocate memory for the extra alignment and the pointer of the
	// position to free (the unaligned pointer)
	char* orig = new char[size + alignment + sizeof(void*)];
	char* aligned = orig + ((((size_t) orig + alignment + sizeof(void*)) & ~(alignment - 1)) - (size_t) orig);
	*((char**) aligned - 1) = orig;
	return aligned;
}

void Shader::bufferFree(void *pointer) const {
	if (pointer == nullptr) {
		return;
	}
	// the offset of the original (unaligned) pointer
	delete[] *((char**) pointer - 1);
}

Id Shader::createKernel(const char *name) {
	core_assert(_program != InvalidId);
	return compute::createKernel(_program, name);
}

void Shader::deleteKernel(Id& kernel) {
	core_assert(_initialized);
	compute::deleteKernel(kernel);
}

bool Shader::loadProgram(const std::string& filename) {
	return loadFromFile(filename + COMPUTE_POSTFIX);
}

bool Shader::loadFromFile(const std::string& filename) {
	const std::string& buffer = core::App::getInstance()->filesystem()->load(filename);
	if (buffer.empty()) {
		return false;
	}
	return load(filename, buffer);
}

std::string Shader::validPreprocessorName(const std::string& name) {
	return core::string::replaceAll(name, "_", "");
}

std::string Shader::getSource(const std::string& buffer, bool finalize) const {
	if (buffer.empty()) {
		return "";
	}
	std::string src;

	core::Var::visitSorted([&] (const core::VarPtr& var) {
		if ((var->getFlags() & core::CV_SHADER) != 0) {
			src.append("#define ");
			const std::string& validName = validPreprocessorName(var->name());
			src.append(validName);
			src.append(" ");
			std::string val;
			if (var->typeIsBool()) {
				val = var->boolVal() ? "1" : "0";
			} else {
				val = var->strVal();
			}
			src.append(val);
			src.append("\n");
		}
	});

	for (auto i = _defines.begin(); i != _defines.end(); ++i) {
		src.append("#ifndef ");
		src.append(i->first);
		src.append("\n");
		src.append("#define ");
		src.append(i->first);
		src.append(" ");
		src.append(i->second);
		src.append("\n");
		src.append("#endif\n");
	}

	std::vector<std::string> includeDirs;
	includeDirs.push_back(std::string(core::string::extractPath(_name)));
	src += util::handleIncludes(buffer, includeDirs);
	int level = 0;
	while (core::string::contains(src, "#include")) {
		src = util::handleIncludes(src, includeDirs);
		++level;
		if (level >= 10) {
			Log::warn("Abort shader include loop for %s", _name.c_str());
			break;
		}
	}

	src = handlePragmas(src);

	core::Var::visitSorted([&] (const core::VarPtr& var) {
		if ((var->getFlags() & core::CV_SHADER) != 0) {
			const std::string& validName = validPreprocessorName(var->name());
			src = core::string::replaceAll(src, var->name(), validName);
		}
	});

	if (finalize) {
		// TODO: do placeholder/keyword replacement
	}
	return src;
}

void Shader::addDefine(const std::string& name, const std::string& value) {
	core_assert_msg(!_initialized, "Shader is already initialized");
	_defines[name] = value;
}

}
