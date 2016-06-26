APP_MODULES := GRT
# APP_STL := stlport_static 
# use this to select gcc instead of clang 
# NDK_TOOLCHAIN_VERSION := 4.8 
# otherwise, the following line select the latest clang version.

 NDK_TOOLCHAIN_VERSION := clang  
APP_STL := gnustl_static

APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
