[![Github Releases](https://img.shields.io/github/release/kubasejdak/liballocator.svg)](https://github.com/kubasejdak/liballocator/releases)
[![Build Status](https://travis-ci.org/kubasejdak/liballocator.svg?branch=master)](https://travis-ci.org/kubasejdak/liballocator)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)
[![codecov.io](http://codecov.io/github/kubasejdak/liballocator/coverage.svg?branch=master)](http://codecov.io/github/kubasejdak/liballocator?branch=master)

# liballocator

The purpose of this project is to provide a convenient and fairly robust memory allocator for the embedded environments in C++. It is based on FreeBSD's UMA (Universal Memory Allocator) called the zone allocator and some concepts taken from SLAB allocator.

## Requirements

Embedded systems have very specific requirements regarding memory allocation. Here is the list of issues/features, that liballocator is trying to address:

* physical memory can be split into multiple continuous spaces,
* boundaries of physical memory space can be specified either by linker script or by programmer in code,
* allocated memory chunks should be aligned to it's size,
* client should be able to overload operator new() to use liballocator,
* allocator should be usable in systems with MMU, MPU or without memory protection,
* there should be low bookkeeping overhead,
* memory shouldn't be subject to fragmentation.

## Overview

liballocator consists of two parts:

* page allocator,
* zone allocator.

## Performance

Tests were performed on macOS Mojave 10.14, Macbook Pro (2,9 GHz Intel Core i5, 8 GB 2133 MHz LPDDR3).

##### Allocate 1000000x 134 bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 3.0761 us | 3.1229 us | 2.2062 us | 2.2090 us |
| malloc | 0.0303 us | 0.0296 us | 0.0008 us | 0.0010 us |
| new | 0.0833 us | 0.0733 us | 0.0008 us | 0.0009 us |

##### Allocate 1000000x 4056 bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 2.9269 us | 3.0897 us | 2.0906 us | 2.1194 us |
| malloc | 0.0300 us | 0.0297 us | 0.0008 us | 0.0006 us |
| new | 0.0832 us | 0.0781 us | 0.0012 us | 0.0008 us |

##### Allocate 2000x random number of bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 16.0200 us | 10.8960 us | 14.8775 us | 9.9780 us |
| malloc | 0.1600 us | 0.2660 us | 0.0600 us | 0.0850 us |
| new | 0.0980 us | 0.2055 us | 0.0020 us | 0.0680 us |