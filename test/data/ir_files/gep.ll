@list = global [ 4 x i32 ] [ i32 1, i32 2, i32 3, i32 4 ]

define i32* @gep() !whyr.ensures !{!{!"war", !"*result == (i32)4"}} {
    %a = getelementptr [ 4 x i32 ], [ 4 x i32 ]* @list, i32 0, i32 3
    ret i32* %a
}

@list2d = global [ 2 x [ 2 x i32 ] ] [ [ 2 x i32 ] [ i32 1, i32 2 ], [ 2 x i32 ] [ i32 3, i32 4 ] ]

define i32* @gep2d() !whyr.ensures !{!{!"war", !"*result == (i32)4"}} {
    %a = getelementptr [ 2 x [ 2 x i32 ] ], [ 2 x [ 2 x i32 ] ]* @list2d, i32 0, i32 1, i32 1
    ret i32* %a
}

@ptr = global i32 0

define i32* @gep_ptr() {
    %a = getelementptr i32, i32* @ptr
    ret i32* %a
}

%simple = type {i32, i16, i8}
%of_arrays = type {[ 2 x i32 ], i32, [ 3 x i32 ]}

@simple_global = global %simple zeroinitializer
define i8* @gep_struct() {
    %a = getelementptr %simple, %simple* @simple_global, i32 0, i32 2
    ret i8* %a
}

@of_arrays_global = global %of_arrays zeroinitializer
define i32* @gep_struct_of_arrays() {
    %a = getelementptr %of_arrays, %of_arrays* @of_arrays_global, i32 0, i32 2, i32 0
    ret i32* %a
}

@of_structs_global = global [ 4 x %simple ] zeroinitializer
define i8* @gep_array_of_structs() {
    %a = getelementptr [ 4 x %simple ], [ 4 x %simple ]* @of_structs_global, i32 0, i32 3, i32 2
    ret i8* %a
}