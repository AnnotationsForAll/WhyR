define i32* @alloca_basic() {
    %a = alloca i32
    ret i32* %a
}

define i32* @alloca_one() {
    %a = alloca i32, i32 1
    ret i32* %a
}

define i32* @alloca_four() {
    %a = alloca i32, i32 4
    ret i32* %a
}

define i32* @alloca_n(i16 %x) {
    %a = alloca i32, i16 %x
    ret i32* %a
}
