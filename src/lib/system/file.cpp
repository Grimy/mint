#include "ast/abstractsyntaxtree.h"
#include "memory/builtin/string.h"
#include "memory/builtin/libobject.h"
#include "memory/memorytool.h"
#include "memory/casttool.h"

#include <stdio.h>

extern "C" {

void mint_file_fopen_2(AbstractSynatxTree *ast) {

	size_t base = get_base(ast);

	Reference &path = *ast->stack().at(base - 1);
	Reference &mode = *ast->stack().at(base);

	Reference *file = Reference::create<LibObject<FILE>>();
	((LibObject<FILE> *)file->data())->impl = fopen(to_string(path).c_str(), to_string(mode).c_str());
	int err = errno;

	ast->stack().pop_back();
	ast->stack().pop_back();

	if (((LibObject<FILE> *)file->data())->impl) {
		ast->stack().push_back(SharedReference::unique(file));
	}
	else {
		ast->stack().push_back(SharedReference());
	}
}

void mint_file_fclose_1(AbstractSynatxTree *ast) {

	Reference &file = *ast->stack().back();

	if (((LibObject<FILE> *)file.data())->impl) {
		fclose(((LibObject<FILE> *)file.data())->impl);
	}
}

void mint_file_readline_1(AbstractSynatxTree *ast) {

	Reference &file = *ast->stack().back();

	int cptr = '\0';
	Reference *result = Reference::create<String>();

	while ((cptr != '\n') && (cptr != EOF)) {
		((String *)result->data())->str += cptr;
		cptr = fgetc(((LibObject<FILE> *)file.data())->impl);
	}

	ast->stack().pop_back();

	if (cptr == EOF) {
		ast->stack().push_back(SharedReference());
	}
	else {
		ast->stack().push_back(SharedReference::unique(result));
	}
}

}
