#include <dlfcn.h>

#ifndef PLUGIN
#define PLUGIN "/usr/local/lib/libmypax.so"
#endif

int
main() {
	void *handle;
	void (*doit)();

	handle = dlopen(PLUGIN, RTLD_LAZY);
	doit = dlsym(handle, "doit");
	(*doit)();
	dlclose(handle);
	return 0;
}
