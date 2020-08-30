#include <ShitHaneul/StackFrame.hpp>

#include <utility>

namespace ShitHaneul {
	StackFrame::StackFrame(const FunctionInfo* currentFunction)
		: m_Top(static_cast<std::size_t>(currentFunction->LocalVariableCount)), m_CurrentFunction(currentFunction) {}
	StackFrame::StackFrame(StackFrame&& stackFrame) noexcept
		: m_Stack(std::move(stackFrame.m_Stack)), m_Top(stackFrame.m_Top),
		m_CurrentFunction(stackFrame.m_CurrentFunction), m_Offset(stackFrame.m_Offset) {}

	StackFrame& StackFrame::operator=(StackFrame&& stackFrame) noexcept {
		m_Stack = std::move(stackFrame.m_Stack);
		m_Top = stackFrame.m_Top;

		m_CurrentFunction = stackFrame.m_CurrentFunction;
		m_Offset = stackFrame.m_Offset;
		return *this;
	}

	void StackFrame::Push(const Constant& constant) {
		m_Stack[m_Top++] = constant;
	}
	Constant StackFrame::Pop() {
		return m_Stack[--m_Top];
	}
	const Constant& StackFrame::GetTop() const noexcept {
		return m_Stack[m_Top - 1];
	}

	void StackFrame::Store(std::uint32_t index) {
		m_Stack[static_cast<std::size_t>(index)] = m_Stack[--m_Top];
	}
	void StackFrame::Load(std::uint32_t index) {
		m_Stack[m_Top++] = m_Stack[static_cast<std::size_t>(index)];
	}
}