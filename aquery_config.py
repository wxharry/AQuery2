# put environment specific configuration here

## GLOBAL CONFIGURATION FLAGS

version_string = '0.4.4a'
add_path_to_ldpath = True
rebuild_backend = False
run_backend = True
have_hge = False
cygroot = 'c:/msys64/usr/bin'
msbuildroot = ''
os_platform = 'unknown'
build_driver = 'Makefile'

def init_config():
    global __config_initialized__, os_platform, msbuildroot
## SETUP ENVIRONMENT VARIABLES
    # __config_initialized__ = False
    #os_platform = 'unkown'
    #msbuildroot = 'd:/gg/vs22/MSBuild/Current/Bin'
    import os
    from engine.utils import add_dll_dir
    # os.environ['CXX'] = 'C:/Program Files/LLVM/bin/clang.exe'
    os.environ['THREADING'] = '1'

    if  ('__config_initialized__' not in globals() or 
            not __config_initialized__):
        import sys
        if os.name == 'nt':
            if sys.platform == 'win32':
                os_platform = 'win'
            elif sys.platform == 'cygwin' or sys.platform == 'msys':
                os_platform = 'cygwin'
        elif os.name == 'posix':
            if sys.platform == 'darwin':
                os_platform = 'mac'
            elif 'linux' in sys.platform:
                os_platform = 'linux'
            elif 'bsd' in sys.platform:
                os_platform = 'bsd'
            elif sys.platform == 'cygwin' or sys.platform == 'msys':
                os_platform = 'cygwin'
        # deal with msys dependencies:
        if os_platform == 'win':
            add_dll_dir(cygroot)  
            add_dll_dir(os.path.abspath('./msc-plugin'))
            import vswhere
            vsloc = vswhere.find(prerelease = True, latest = True, prop = 'installationPath')
            if vsloc:
                msbuildroot = vsloc[0] + '/MSBuild/Current/Bin/MSBuild.exe'
            else:
                print('Warning: No Visual Studio installation found.')
            # print("adding path")
        else:
            import readline
            if os_platform == 'cygwin':
                add_dll_dir('./lib')
        __config_initialized__ = True
        
