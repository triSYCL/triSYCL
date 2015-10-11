# Jacobi examples

This directory contains four Jacobi examples in 2 dimensions using triSYCL. Two
are based directly on triSYCL, a simple version and a complex one with tiling.
The two others examples (suffixed by -st) are based on a stencil DSEL, itself
based on triSYCL. It enables to have the same #LOC as the simple jacobi version
but using the tiling technique underlaid over the DSEL.

## About Jacobi

It's a 5-points non-convergent stencil. Two arrays are used : the computations
results on the first array are stored in the second. Then the latter is copied
back in the first array (there is no swap). This operation is reproduced 3 times
but it can be changed by compilation directives or by arguments :

[NB_ITER PB_SIZE0 PB_SIZE1 [TILE_SIZE0 TILE_SIZE1] ]

Be aware that PB_SIZE must include ghosts cells : 2 more in this case.

## About the stencil DSEL

Just read the jacobi-st-* to have an idea ! It enables to describe a stencil by
its coefficients (fixed or variables wrt. their positions in the stencil or the 
global array). The user must write itself the display layout functions to access
the elements through the SYCL accessors.

Some improvments are needed, two are really importants : 
- templatization of the binary operation between a coefficient and an element;
- adaptation to non 2D stencils. 

The first implies to have a template argument as 
```
std::function<T(T,T)> = T::operator+
```
but this must be an inner template as in C++ a type specified in a template
cannot be use by the other argument of the template. The second is not esay
since we want avoid rewrite all the code for each dimension case. So we need
a way to store and access an arbitrary number of const int in a template argument
, which is maybe possible with _variadic template_. And we need an adaptater to
reduce higher dimensions to the maximum of 3 allowed in SYCL.

Feel free to propose your ideas !
