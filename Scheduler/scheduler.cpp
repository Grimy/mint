#include "Scheduler/scheduler.h"
#include "Compiler/compiler.h"

#include <cstring>

using namespace std;

Scheduler *Scheduler::g_instance = nullptr;

Scheduler::Scheduler(int argc, char **argv) : m_running(false), m_status(0) {

	g_instance = this;

	parseArguments(argc, argv);
}

Scheduler::~Scheduler() {

	g_instance = nullptr;

	for (auto thread : m_threads) {
		delete thread;
	}

	Module::clearCache();
}

Scheduler *Scheduler::instance() {
	return g_instance;
}

int Scheduler::run() {

	if (m_threads.empty()) {
		m_threads.push_back(Process::readInput());
	}

	m_running = true;

    while (!m_threads.empty()) {
		for (auto thread = m_threads.begin(); thread != m_threads.end(); ++thread) {

			Process *process = *thread;

			if (!process->exec(42)) {
				if (isOver()) {
					return m_status;
				}
				else if (process->isOver()) {
					delete process;
					thread = m_threads.erase(thread);
				}
			}
		}

		GarbadgeCollector::free();
	}

	return m_status;
}

void Scheduler::exit(int status) {
	m_status = status;
	m_running = false;
}

bool Scheduler::isOver() const {
	return !m_running;
}

void Scheduler::parseArguments(int argc, char **argv) {

	/// \todo handle first arg

	for (int argn = 1; argn < argc; argn++) {
		if (!parseArgument(argc, argn, argv)) {
			/// \todo raise error
		}
	}

}

bool Scheduler::parseArgument(int argc, int &argn, char **argv) {

	if (!strcmp(argv[argn], "--version")) {
		if (argn == 1) {
			printVersion();
			exit(0);
		}
	}
	else if (!strcmp(argv[argn], "--help")) {
		if (argn == 1) {
			printHelp();
			exit(0);
		}
	}
	else {
		m_threads.push_back(Process::create(argv[argn]));
		return true;
	}

	return false;
}

void Scheduler::printVersion() {

}

void Scheduler::printHelp() {

}
