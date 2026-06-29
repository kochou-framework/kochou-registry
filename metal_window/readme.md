# Metal Window
Component provides window for MacOS following common window interface (see ```kochou-registry/common/window_interface.hpp```).

Depency on target OS component will use different implementation:
* MacOS --- real Objective-C implementation
* Other --- dummy C++ implementation

### Component requirements
* ```ktl::api::extension::khr_surface```
* ```ktl::api::extension::ext_metal_surface```
* ```ktl::api::extension::khr_portability_subset```
* ```ktl::api::extension::khr_portability_enumeration```

