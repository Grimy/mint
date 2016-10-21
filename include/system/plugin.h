#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>

class AbstractSynatxTree;

class Plugin {
public:
	Plugin(const std::string &path);
	~Plugin();

	static Plugin *load(const std::string &plugin);

	bool call(const std::string &function, AbstractSynatxTree *ast);

protected:
#ifdef _WIN32

#else
	typedef void *handle_type;
#endif
	typedef void (*function_type)(AbstractSynatxTree *ast);

private:
	handle_type m_handle;
};

#endif // PLUGIN_H