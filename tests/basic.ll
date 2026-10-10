define i32 @constant_case() {
entry:
  %a = add i32 10, 20
  %b = mul i32 3, 4
  %c = add i32 %a, %b
  ret i32 %c
}

define i32 @identity_case(i32 %x) {
entry:
  %a = add i32 %x, 0
  %b = mul i32 %a, 1
  ret i32 %b
}

define i32 @cse_case(i32 %x, i32 %y) {
entry:
  %a = add i32 %x, %y
  %b = add i32 %x, %y
  %c = mul i32 %a, 2
  %d = add i32 %b, %c
  ret i32 %d
}

define i32 @dead_case(i32 %x) {
entry:
  %unused = add i32 %x, 99
  ret i32 %x
}
