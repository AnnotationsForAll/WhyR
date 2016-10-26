define <4 x i32> @vtrunc() {
    %a = trunc <4 x i64> <i64 8, i64 2, i64 77, i64 0> to <4 x i32>
    ret <4 x i32> %a
}
