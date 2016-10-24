%one_long = type {i64}
%two_ints = type {i32, i32}
%mixed_ints = type {i8, i16, i32}

@mixed_types = global { i32, i16*, [ 2 x %two_ints ], float } { i32 42, i16* null, [ 2 x %two_ints ] [ %two_ints { i32 1, i32 2 }, %two_ints zeroinitializer ], float 1.0}

define void @structs() {
    %a = alloca %one_long
    %b = alloca %two_ints
    %c = alloca %mixed_ints
    ret void
}
