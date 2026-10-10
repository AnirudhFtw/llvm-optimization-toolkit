define i32 @constant_branch(i32 %x) {
entry:
  br i1 true, label %yes, label %no

yes:
  %a = add i32 %x, 1
  ret i32 %a

no:
  %b = add i32 %x, 2
  ret i32 %b
}
