project (address_spaces)

declare_trisycl_test(TARGET address_spaces TEST_REGEX
"i = 3
f\\[0\\] = 2
f\\[0\\] = 2
f\\[1\\] = 3
f\\[1\\] = 3
f\\[0\\] = 2
\\*c_p = s
global_float = 6
Range = {2,3}
Range2 = {2,3}
s = yo man Size = 6
ps = yo manyo man Size = 12
e-salut
e-= Mont a ra mat\\?
3")
declare_trisycl_test(TARGET address_spaces_pstl)
declare_trisycl_test(TARGET address_spaces_ptr)
