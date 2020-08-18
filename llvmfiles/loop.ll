%s1 = alloca i32
%s2 = load i32, i32* %s1
br L6
%s3 = load i32, i32* %s1
%s5 = icmp slt i32 %s2, 5
%s0 = %s0
%s7 = add i32 %s2, 1
store i32 %s7, i32* %s1
%s8 = load i32, i32* %s1
br L4
