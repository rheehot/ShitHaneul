#include <ShitHaneul/Parser.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <ios>
#include <memory>
#include <utility>

namespace ShitHaneul {
	ByteFile::ByteFile(ByteFile&& byteFile) noexcept
		: m_FunctionInfos(std::move(byteFile.m_FunctionInfos)), m_Functions(std::move(byteFile.m_Functions)),
		m_RootFunction(byteFile.m_RootFunction) {}
	ByteFile::~ByteFile() {
		Clear();
	}

	ByteFile& ByteFile::operator=(ByteFile&& byteFile) noexcept {
		m_FunctionInfos = std::move(byteFile.m_FunctionInfos);
		m_Functions = std::move(byteFile.m_Functions);
		m_RootFunction = byteFile.m_RootFunction;
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
	const Function* ByteFile::GetRoot() const noexcept {
		return m_RootFunction;
	}
	Function* ByteFile::GetRoot() noexcept {
		return m_RootFunction;
	}
	void ByteFile::SetRoot(Function* function) noexcept {
		m_RootFunction = function;
	}
}

namespace ShitHaneul {
	bool Parser::Load(const std::string& path) {
		std::ifstream stream(path, std::ifstream::binary);
		if (!stream) return false;

		stream.seekg(0, std::ifstream::end);
		const std::streamsize length = static_cast<std::streamsize>(stream.tellg());
		stream.seekg(0, std::ifstream::beg);

		m_Bytes.resize(static_cast<std::size_t>(length));
		stream.read(reinterpret_cast<char*>(m_Bytes.data()), length);
		if (stream.gcount() != length) return false;

		m_Cursor = 0;
		m_Result.Clear();
		return true;
	}
	void Parser::Parse() {
		m_Result.SetRoot(ParseFunction());
	}
	ByteFile Parser::GetResult() noexcept {
		return std::move(m_Result);
	}

	char32_t Parser::ReadCharacter() {
		const auto first = ReadScalar<std::uint8_t>();
		if (first < 0x80) return first;
		else if (first < 0xE0) {
			const auto second = ReadScalar<std::uint8_t>();
			return ((static_cast<char32_t>(first) & 0x1F) << 6) + (static_cast<char32_t>(second) & 0x3F);
		} else if (first < 0xF0) {
			const auto second = ReadScalar<std::uint8_t>();
			const auto third = ReadScalar<std::uint8_t>();
			return ((static_cast<char32_t>(first) & 0x0F) << 12) + ((static_cast<char32_t>(second) & 0x3F) << 6) +
				(static_cast<char32_t>(third) & 0x3F);
		} else {
			const auto second = ReadScalar<std::uint8_t>();
			const auto third = ReadScalar<std::uint8_t>();
			const auto fourth = ReadScalar<std::uint8_t>();
			return ((static_cast<char32_t>(first) & 0x07) << 18) + ((static_cast<char32_t>(second) & 0x3F) << 12) +
				((static_cast<char32_t>(second) & 0x3F) << 6) + (static_cast<char32_t>(fourth) & 0x3F);
		}
	}

	Function* Parser::ParseFunction() {
		std::unique_ptr<FunctionInfo> info(new FunctionInfo);
		info->JosaList = ParseStringList();
		info->GlobalList = ParseGlobalList();
		info->StackOperandCount = ReadScalar<std::uint64_t>();
		info->LocalVariableCount = ReadScalar<std::uint32_t>();
		info->ConstantList = ParseConstantList();
		info->Name = ReadString();
		info->Line.Path = ReadString();
		info->Line.Line = ReadScalar<std::uint16_t>();
		info->LineMap = ParseLineMap();
		info->InstructionList = ParseInstructionList();

		Function* const result = m_Result.RegisterFunction(info.get());
		return info.release(), result;
	}
	StringList Parser::ParseStringList() {
		StringList result;
		const auto count = ReadScalar<std::uint8_t>();
		result.Reserve(count);

		for (std::uint8_t i = 0; i < count; ++i) {
			result.Add(ReadString<std::uint8_t>());
		}
		return result;
	}
	std::vector<std::u32string> Parser::ParseGlobalList() {
		std::vector<std::u32string> result;
		const auto count = ReadScalar<std::uint64_t>();
		result.reserve(static_cast<std::size_t>(count));

		for (std::uint64_t i = 0; i < count; ++i) {
			result.push_back(ReadString());
		}
		return result;
	}
	ConstantList Parser::ParseConstantList() {
		ConstantList result;
		const auto count = ReadScalar<std::uint64_t>();
		result.Reserve(count);

		for (std::uint64_t i = 0; i < count; ++i) {
			const auto type = ReadScalar<Type>();
			switch (type) {
			case Type::None:
				result.Add();
				break;

			case Type::Integer:
				result.Add(ReadScalar<std::int64_t>());
				break;

			case Type::Real:
				result.Add(ReadScalar<double>());
				break;

			case Type::Boolean:
				result.Add(ReadScalar<bool>());
				break;

			case Type::Character:
				result.Add(ReadCharacter());
				break;

			case Type::Function:
				result.Add(ParseFunction());
				break;
			}
		}
		return result;
	}
	LineMap Parser::ParseLineMap() {
		LineMap result;
		const auto count = ReadScalar<std::uint64_t>();
		result.Reserve(count);

		for (std::uint64_t i = 0; i < count; ++i) {
			const auto offset = ReadScalar<std::uint32_t>();
			const auto isLine = ReadScalar<bool>();
			if (isLine) {
				result.Add(offset, ReadScalar<std::uint16_t>());
			} else {
				result.Add(offset, ReadString());
			}
		}
		return result;
	}
	InstructionList Parser::ParseInstructionList() {
		InstructionList result;
		const auto count = ReadScalar<std::uint64_t>();
		result.Reserve(count);

		for (std::uint64_t i = 0; i < count; ++i) {
			result.Add(ParseInstruction());
		}
		return result;
	}
	Instruction Parser::ParseInstruction() {
		Instruction result;
		result.OpCode = ReadScalar<OpCode>();

		switch (result.OpCode) {
		case OpCode::Push:
		case OpCode::LoadLocal:
		case OpCode::StoreLocal:
		case OpCode::LoadDeref:
		case OpCode::StoreGlobal:
		case OpCode::LoadGlobal:
		case OpCode::PopJmpIfFalse:
		case OpCode::Jmp:
			result.Operand = ReadScalar<std::uint32_t>();
			break;

		case OpCode::FreeVar:
			result.Operand = ParseFreeVariableList();
			break;

		case OpCode::Call:
			result.Operand = ParseStringList();
			break;

		case OpCode::AddStruct:
		case OpCode::MakeStruct: {
			auto name = ReadString<std::uint8_t>();
			result.Operand = std::make_pair(std::move(name), ParseStringList());
			break;
		}

		case OpCode::GetField:
			result.Operand = ReadString<std::uint8_t>();
			break;
		}
		return result;
	}
	FreeVariableList Parser::ParseFreeVariableList() {
		FreeVariableList result;
		const auto count = ReadScalar<std::uint8_t>();
		result.reserve(static_cast<std::size_t>(count));

		for (std::uint8_t i = 0; i < count; ++i) {
			const auto variableType = ReadScalar<VariableType>();
			const auto index = ReadScalar<std::uint8_t>();
			result.push_back({ variableType, index });
		}
		return result;
	}
}