---
title: atomic
---

{{< callout type="info">}}
  Header: `atomic.h`  
  Since: `TBC`  
  Similar to: [atomic](https://en.cppreference.com/cpp/atomic/atomic)
{{< /callout >}}

This header attempts to replicate some of the types from `std::atomic`.  

If `ETL_CPP11_SUPPORTED` is defined as `1` in the profile then `etl::atomic` will be defined in terms of `std::atomic`.  
Otherwise it will be implemented in terms of the built-in support, if available, from the compiler. For example, early GCC and Arm compilers will use the `__sync` built-ins.  

If a type cannot be handled by the built-ins then the types are wrapped by `etl::mutex`.  

If there is an ETL atomic type available for your platform then `ETL_HAS_ATOMIC` will be set to `1`, otherwise
it will be set to `0`.  

From: `20.40.0`  `etl::atomic` supports the `is_always_lock_free` property.  

Define `ETL_ATOMIC_FORCE_ETL_IMPLEMENTATION` to select the ETL implementation of `etl::atomic` (using the compiler built-ins) in preference to `std::atomic`, even when the STL is available.
This is only supported for the Arm, GCC and Clang compilers.  
