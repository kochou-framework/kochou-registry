# kochou-registry
Is a public repositoty that contains approwed engine/renderer components.

### Components

| ID | Name | Version | Description |
|:---|:-----|:--------|:------------|
| 0 | Example Dynamic Pipeline | 1.0.0 | Provides default (base implementation) configuration of dynamic Vulkan pipeline |
| 1 | Example Dynamic Render | 1.0.0 | Provides default (base implementation) configuration of dynamic Vulkan render |
| 2 | Metal Window | 1.0.0 | Provides window for MacOS (Quartz, Cocoa, Metal) following common window interface |
| 3 | Xplat Window | in dev | Provides crossplatform window following common window interface |

### Common component tree:
```txt
components (dir)
-- kochou_component_XXX
    -- readme.md
    -- spec.json
    -- src (dir, cpp)
    -- include (dir, hpp)
    -- modules (dir, cppm)
    -- xmake.lua
```

### Public custom component
You can develop and public your own component following next rules:
* Support framework requirements system (see kochou/requirements)
* Component tree must correspond the template
After filling all auxiliary files (spec.json, readme.md, ...) you can make a pull request!
