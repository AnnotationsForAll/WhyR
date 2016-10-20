@ptr = global i32 0

define i64 @ptr_to_i64() {
    %a = ptrtoint i32* @ptr to i64
    ret i64 %a
}

define i32 @ptr_to_i32() {
    %a = ptrtoint i32* @ptr to i32
    ret i32 %a
}

define i8* @i64_to_ptr() {
    %a = inttoptr i64 777 to i8*
    ret i8* %a
}

define i8* @i32_to_ptr() {
    %a = inttoptr i32 777 to i8*
    ret i8* %a
}

@list = global [ 2 x i32 ] [ i32 4, i32 2 ]

define i32* @front() {
    %a = bitcast [ 2 x i32 ]* @list to i32*
    ret i32* %a
}
