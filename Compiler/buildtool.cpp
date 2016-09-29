#include "Compiler/buildtool.h"
#include "AbstractSyntaxTree/module.h"
#include "Memory/object.h"
#include "Memory/class.h"
#include "System/error.h"

using namespace std;

BuildContext::BuildContext(DataStream *stream, Module::Context node) :
	lexer(stream), data(node) {
	m_global = false;
}

void BuildContext::beginLoop() {

	Loop ctx;

	ctx.backward = &m_jumpBackward.top();
	ctx.forward = &m_jumpForward.top();

	m_loops.push(ctx);
}

void BuildContext::endLoop() {
	m_loops.pop();
}

bool BuildContext::isInLoop() const {
	return !m_loops.empty();
}

void BuildContext::startJumpForward() {

	m_jumpForward.push({data.module->nextInstructionOffset()});
	pushInstruction(0);
}

void BuildContext::loopJumpForward() {
	m_loops.top().forward->push_back(data.module->nextInstructionOffset());
	pushInstruction(0);
}

void BuildContext::shiftJumpForward() {

	auto firstLabel = m_jumpForward.top();
	m_jumpForward.pop();

	auto secondLabel = m_jumpForward.top();
	m_jumpForward.pop();

	m_jumpForward.push(firstLabel);
	m_jumpForward.push(secondLabel);
}

void BuildContext::resolveJumpForward() {

	Instruction instruction;
	instruction.parameter = data.module->nextInstructionOffset();
	for (size_t offset : m_jumpForward.top()) {
		data.module->replaceInstruction(offset, instruction);
	}
	m_jumpForward.pop();
}

void BuildContext::startJumpBackward() {

	m_jumpBackward.push(data.module->nextInstructionOffset());
}

void BuildContext::loopJumpBackward() {
	pushInstruction(*m_loops.top().backward);
}

void BuildContext::shiftJumpBackward() {

	auto firstLabel = m_jumpBackward.top();
	m_jumpBackward.pop();

	auto secondLabel = m_jumpBackward.top();
	m_jumpBackward.pop();

	m_jumpBackward.push(firstLabel);
	m_jumpBackward.push(secondLabel);
}

void BuildContext::resolveJumpBackward() {

	pushInstruction(m_jumpBackward.top());
	m_jumpBackward.pop();
}

void BuildContext::startDefinition() {
	Definition *def = new Definition;
	def->function = data.module->makeConstant(Reference::alloc<Function>());
	def->beginOffset = data.module->nextInstructionOffset();
	def->variadic = false;
	m_definitions.push(def);
}

bool BuildContext::addParameter(const string &symbol) {

	Definition *def = m_definitions.top();
	if (def->variadic) {
		parse_error("unexpected parameter after '...' token");
		return false;
	}

	def->parameters.push(symbol);
	return true;
}

bool BuildContext::setVariadic() {

	Definition *def = m_definitions.top();
	if (def->variadic) {
		parse_error("unexpected parameter after '...' token");
		return false;
	}

	def->parameters.push("va_args");
	def->variadic = true;
	return true;
}

bool BuildContext::saveParameters() {

	Definition *def = m_definitions.top();
	if (def->variadic && def->parameters.empty()) {
		parse_error("expected parameter before '...' token");
		return false;
	}

	int signature = def->variadic ? -(def->parameters.size() - 1) : def->parameters.size();
	((Function *)def->function->data())->mapping.insert({signature, {data.moduleId, def->beginOffset}});

	while (!def->parameters.empty()) {
		pushInstruction(Instruction::init_param);
		pushInstruction(def->parameters.top().c_str());
		def->parameters.pop();
	}

	return true;
}

bool BuildContext::addDefinitionSignature() {

	Definition *def = m_definitions.top();
	if (def->variadic) {
		parse_error("unexpected parameter after '...' token");
		return false;
	}

	int signature = def->parameters.size();
	((Function *)def->function->data())->mapping.insert({signature, {data.moduleId, def->beginOffset}});
	def->beginOffset = data.module->nextInstructionOffset();
	return true;
}

void BuildContext::saveDefinition() {

	Instruction instruction;
	Definition *def = m_definitions.top();

	instruction.constant = def->function;
	pushInstruction(Instruction::load_constant);
	data.module->pushInstruction(instruction);
	m_definitions.pop();
	delete def;
}

Data *BuildContext::retriveDefinition() {

	Definition *def = m_definitions.top();
	Data *data = def->function->data();

	m_definitions.pop();
	delete def;

	return data;
}

void BuildContext::startClassDescription(const string &name) {
	m_classDescription.push(new Class(name));
}

void BuildContext::classInheritance(const string &parent) {
	m_classDescription.top().addParent(parent);
}

void BuildContext::addMember(Reference::Flags flags, const string &name, Data *value) {

	if (m_global) {
		m_classDescription.top().addGlobalMember(name, SharedReference::unique(new Reference(flags, value)));
		m_global = false;
	}
	else {
		m_classDescription.top().addMember(name, SharedReference::unique(new Reference(flags, value)));
	}
}

void BuildContext::resolveClassDescription() {

	ClassDescription desc = m_classDescription.top();
	m_classDescription.pop();

	if (m_classDescription.empty()) {
		pushInstruction(Instruction::register_class);
		pushInstruction(GlobalData::instance().createClass(desc));
	}
	else {
		m_classDescription.top().addMemberClass(desc);
	}
}

void BuildContext::startCall() {
	m_calls.push(0);
}

void BuildContext::addToCall() {
	m_calls.top()++;
}

void BuildContext::resolveCall() {
	pushInstruction(m_calls.top());
	m_calls.pop();
}

void BuildContext::pushInstruction(Instruction::Command command) {

	Instruction instruction;
	instruction.command = command;
	data.module->pushInstruction(instruction);
}

void BuildContext::pushInstruction(int parameter) {

	Instruction instruction;
	instruction.parameter = parameter;
	data.module->pushInstruction(instruction);
}

void BuildContext::pushInstruction(const char *symbol) {

	Instruction instruction;
	instruction.symbol = data.module->makeSymbol(symbol);
	data.module->pushInstruction(instruction);
}

void BuildContext::pushInstruction(Data *constant) {

	Instruction instruction;
	instruction.constant = data.module->makeConstant(constant);
	data.module->pushInstruction(instruction);
}

void BuildContext::setModifiers(Reference::Flags flags) {
	m_modifiers = flags;
}

Reference::Flags BuildContext::getModifiers() const {
	return m_modifiers;
}

void BuildContext::setGlobal(bool global) {
	m_global = global;
}

void BuildContext::parse_error(const char *error_msg) {

#ifndef _WIN32
	fprintf(stderr, "\033[1;31m");
#endif
	fprintf(stderr, "%s:%lu %s\n", lexer.path().c_str(), lexer.lineNumber(), error_msg);
#ifndef _WIN32
	fprintf(stderr, "\033[0m");
#endif

	fprintf(stderr, "%s\n", lexer.lineError().c_str());

	fflush(stdout);
}
