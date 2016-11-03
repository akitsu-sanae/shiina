# Shiina is ...

Shiina is C++ lightweight JSON parser.  

# How to use
Shiina is a header only library.  
All you must do is `#include "shiina.hpp"`

# Usage

```cpp
auto obj = shiina::parse(R"({"pi": 3.141592})");
std::cout << obj << std::endl; // printed "{hoge: pi}"
std::cout << obj["hoge"] << std::endl; // printed "3.141592"
```

# Copyright
Copyright (C) 2016 akitsu sanae.  
Distributed under the Boost Software License, Version 1.0. 
(See accompanying file LICENSE or copy at http://www.boost/org/LICENSE_1_0.txt)  


