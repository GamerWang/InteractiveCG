//-------------------------------------------------------------------------------
///
/// \file Main.cpp
///	\author Xipeng Wang
/// \version 1.0
/// \date 01/18/2020
///
///	\Main program of the Renderer
///
//-------------------------------------------------------------------------------

#define default_obj_name "teapot.obj"
#define enable_default_obj

//-------------------------------------------------------------------------------

void ShowViewport(int argc, char* argv[]);

//-------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
#ifdef enable_default_obj
	if (argc <= 1) {
		argc = 2;
		char defaultObjName[] = default_obj_name;
		argv[1] = defaultObjName;
	}
#endif

	ShowViewport(argc, argv);
	return 0;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------