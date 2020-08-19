%s1 = alloca i32
%s2 = load i32, i32* %s1
store i32 0, i32* %s1
%s3 = load i32, i32* %s1
br label %L4
L4:
%s5 = icmp slt i32 %s2, 5
br i1 %s5, label %L6, label %L9
L6:
%s7 = add i32 %s2, 1
store i32 %s7, i32* %s1
%s8 = load i32, i32* %s1
br label %L4
L9:
