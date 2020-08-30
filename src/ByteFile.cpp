#include <ShitHaneul/ByteFile.hpp>

#include <algorithm>
#include <memory>

namespace ShitHaneul {
	ByteFile::ByteFile(ByteFile&& byteFile) noexcept
		: m_Functions(std::move(byteFile.m_Functions)) {}
	ByteFile::~ByteFile() {
		Clear();
	}

	ByteFile& ByteFile::operator=(ByteFile&& byteFile) noexcept {
		m_Functions = std::move(byteFile.m_Functions);
		return *this;
	}

	void ByteFile::Clear() noexcept {
		static constexpr auto deleter = [](auto pointer) {
			delete pointer;
		};

		std::for_each(m_FunctionInfos.begin(), m_FunctionInfos.end(), deleter);
		std::for_each(m_Functions.begin(), m_Functions.end(), deleter);
	}

	Function* ByteFile::RegisterFunction(FunctionInfo* functionInfo) {
		m_FunctionInfos.push_back(functionInfo);

		std::unique_ptr<Function> result(new Function(functionInfo));
		AddFunction(result.get());
		return result.release();
	}
	void ByteFile::AddFunction(Function* function) {
		m_Functions.push_back(function);
	}
}