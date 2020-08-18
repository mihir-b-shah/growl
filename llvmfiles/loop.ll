%s1 = alloca i32
%s2 = load i32, i32* %s1
store i32 0, i32* %s1
%s3 = load i32, i32* %s1
%s4 = icmp slt i32 %s2, 5
