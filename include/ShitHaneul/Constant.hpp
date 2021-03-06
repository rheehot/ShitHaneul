#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ShitHaneul {
	enum class Type : std::uint8_t {
		None,
		Integer,
		Real,
		Boolean,
		Character,
		Function,
		Structure,
	};

	static constexpr std::string_view TypeName[] = {
		"없음", "정수", "실수", "부울", "문자", "함수", "구조체"
	};
}

namespace ShitHaneul {
	class Function;
	class StringMap;

	class NoneConstant final {
	public:
		NoneConstant() noexcept = default;
		NoneConstant(const NoneConstant& constant) noexcept;
		~NoneConstant() = default;

	public:
		NoneConstant& operator=(const NoneConstant& constant) noexcept;
	};

	class IntegerConstant final {
	public:
		std::int64_t Value = 0;

	public:
		IntegerConstant() noexcept = default;
		IntegerConstant(std::int64_t value) noexcept;
		IntegerConstant(const IntegerConstant& constant) noexcept;
		~IntegerConstant() = default;

	public:
		IntegerConstant& operator=(const IntegerConstant& constant) noexcept;
	};

	class RealConstant final {
	public:
		double Value = 0.0;

	public:
		RealConstant() noexcept = default;
		RealConstant(double value) noexcept;
		RealConstant(const RealConstant& constant) noexcept;
		~RealConstant() = default;

	public:
		RealConstant& operator=(const RealConstant& constant) noexcept;
	};

	class BooleanConstant final {
	public:
		bool Value = false;

	public:
		BooleanConstant() noexcept = default;
		BooleanConstant(bool value) noexcept;
		BooleanConstant(const BooleanConstant& constant) noexcept;
		~BooleanConstant() = default;

	public:
		BooleanConstant& operator=(const BooleanConstant& constant) noexcept;
	};

	class CharacterConstant final {
	public:
		char32_t Value = '\0';

	public:
		CharacterConstant() noexcept = default;
		CharacterConstant(char32_t value) noexcept;
		CharacterConstant(const CharacterConstant& constant) noexcept;
		~CharacterConstant() = default;

	public:
		CharacterConstant& operator=(const CharacterConstant& constant) noexcept;
	};

	class FunctionConstant final {
	public:
		Function* Value = nullptr;

	public:
		FunctionConstant() noexcept = default;
		FunctionConstant(Function* value) noexcept;
		FunctionConstant(const FunctionConstant& constant) noexcept;
		~FunctionConstant() = default;

	public:
		FunctionConstant& operator=(const FunctionConstant& constant) noexcept;
	};

	class StructureConstant final {
	public:
		StringMap* Value = nullptr;

	public:
		StructureConstant() noexcept = default;
		StructureConstant(StringMap* value) noexcept;
		StructureConstant(const StructureConstant& constant) noexcept;
		~StructureConstant() = default;

	public:
		StructureConstant& operator=(const StructureConstant& constant) noexcept;
	};
}

namespace ShitHaneul {
	using Constant = std::variant<std::monostate,
		NoneConstant,
		IntegerConstant,
		RealConstant,
		BooleanConstant,
		CharacterConstant,
		FunctionConstant,
		StructureConstant>;

	Type GetType(const Constant& constant) noexcept;

	template<typename T>
	using MakeConstantClass = std::conditional_t<
		std::is_same_v<T, std::int64_t>, IntegerConstant, std::conditional_t<
		std::is_same_v<T, double>, RealConstant, std::conditional_t<
		std::is_same_v<T, bool>, BooleanConstant, std::conditional_t<
		std::is_same_v<T, char32_t>, CharacterConstant, std::conditional_t<
		std::is_same_v<T, Function*>, FunctionConstant, std::conditional_t<
		std::is_same_v<T, StringMap*>, StructureConstant,
		void
	>>>>>>;

	class ConstantList final {
	private:
		std::vector<Constant> m_List;

	public:
		ConstantList() noexcept = default;
		ConstantList(ConstantList&& constantList) noexcept;
		~ConstantList() = default;

	public:
		ConstantList& operator=(ConstantList&& constantList) noexcept;
		Constant operator[](std::uint64_t index) const noexcept;

	public:
		void Add();
		template<typename T>
		void Add(T rawConstant);
		std::uint64_t GetCount() const noexcept;
		void Reserve(std::uint64_t count);
	};
}

namespace ShitHaneul {
	class StringList final {
	private:
		std::vector<std::pair<std::size_t, std::u32string>> m_List;

	public:
		StringList() noexcept = default;
		StringList(StringList&& stringList) noexcept;
		~StringList() = default;

	public:
		StringList& operator=(StringList&& stringList) noexcept;
		const std::pair<std::size_t, std::u32string>& operator[](std::uint8_t index) const noexcept;

	public:
		void Add(std::u32string&& string);
		std::uint8_t GetCount() const noexcept;
		void Reserve(std::uint8_t count);
	};
}

namespace ShitHaneul {
	class StringMap final {
	private:
		std::vector<std::pair<std::pair<std::size_t, std::u32string_view>, Constant>> m_Map;

	public:
		explicit StringMap(const StringList& stringList);
		StringMap(const StringMap& stringMap) noexcept;
		StringMap(StringMap&& stringMap) noexcept;
		~StringMap() = default;

	public:
		StringMap& operator=(const StringMap& stringMap) noexcept;
		StringMap& operator=(StringMap&& stringMap) noexcept;

	public:
		void BindConstant(const std::u32string_view& string, const Constant& constant);
	};
}

#include "impl/Constant.hpp"