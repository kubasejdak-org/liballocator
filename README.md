[![Github Releases](https://img.shields.io/github/release/ksejdak/liballocator.svg)](https://github.com/ksejdak/liballocator/releases)
[![Build Status](https://travis-ci.org/ksejdak/liballocator.svg?branch=master)](https://travis-ci.org/ksejdak/liballocator)

# liballocator

The purpose of this project is to provide a convenient and fairly robust memory allocator for embedded environments in C++. It is based on FreeBSD's UMA (Universal Memory Allocator) called the zone allocator and some concepts taken from SLAB allocator.

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
