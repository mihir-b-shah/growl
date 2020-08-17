%s1 = alloca i32
%s4 = load i32, i32* %s1
store i32 7, i32* %s1
%s5 = load i32, i32* %s1
%s2 = alloca i32
%s6 = load i32, i32* %s2
%s3 = alloca i32
%s7 = load i32, i32* %s3
store i32 99, i32* %s2
%s8 = load i32, i32* %s2
%s9 = add i32 %s4, %s6
store i32 %s9, i32* %s3
%s10 = load i32, i32* %s3
