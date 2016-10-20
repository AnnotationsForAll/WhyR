define void @main() {
    LoopHeader:
    br label %Loop
    Loop:
    %indvar = phi i32 [ 0, %LoopHeader ], [ %nextindvar, %Loop ]
    %nextindvar = add i32 %indvar, 1
    br label %Loop, !whyr.assert !{!{!"war", !"%indvar sge (i32)0"}}
}