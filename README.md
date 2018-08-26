[![Github Releases](https://img.shields.io/github/release/kubasejdak/liballocator.svg)](https://github.com/kubasejdak/liballocator/releases)
[![Build Status](https://travis-ci.org/kubasejdak/liballocator.svg?branch=master)](https://travis-ci.org/kubasejdak/liballocator)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)
[![codecov.io](http://codecov.io/github/kubasejdak/liballocator/coverage.svg?branch=master)](http://codecov.io/github/kubasejdak/liballocator?branch=master)

# liballocator

The purpose of this project is to provide a convenient and fairly robust memory allocator for the embedded environments in C++. It is based on FreeBSD's UMA (Universal Memory Allocator) called the zone allocator and some concepts taken from SLAB allocator.

## Requirements

Embedded systems have very specific requirements regarding memory allocation. Here is the list of issues/features, that liballocator is trying to address:

* physical memory can be split into multiple continuous spaces,
* boundaries of physical memory space can be specified either by linker script or by programmer,
* allocated memory chunks should be aligned to it's size,
* client should be able to overload operator new() to use zone-allocator,
* allocator should be usable in systems with MMU, MPU or without memory protection,
* there should be low bookkeeping overhead,
* memory shouldn't be subject to fragmentation.

## Overview

liballocator consists of two parts:

* page allocator,
* zone allocator.
