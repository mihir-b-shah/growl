%s1 = alloca i32
%s2 = load i32, i32* %s1
store i32 0, i32* %s1
%s3 = load i32, i32* %s1
%s4 = icmp ult i64 %s2, 11
%s5 = icmp ne i64 %s4 0
