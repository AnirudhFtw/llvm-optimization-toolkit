target triple = "nvptx64-nvidia-cuda"

declare i32 @llvm.nvvm.read.ptx.sreg.tid.x()
declare i32 @llvm.nvvm.read.ptx.sreg.ctaid.x()
declare i32 @llvm.nvvm.read.ptx.sreg.ntid.x()

define ptx_kernel void @compute(ptr addrspace(1) %out, i32 %N) {
entry:
    %tid = call i32 @llvm.nvvm.read.ptx.sreg.tid.x()
    %bid = call i32 @llvm.nvvm.read.ptx.sreg.ctaid.x()
    %bdim = call i32 @llvm.nvvm.read.ptx.sreg.ntid.x()
    %base = mul i32 %bid, %bdim
    %idx = add i32 %base, %tid
    %valid = icmp ult i32 %idx, %N
    br i1 %valid, label %body, label %exit

body:
    %bias = add i32 10, 20
    %result = add i32 %idx, %bias
    %idx64 = zext i32 %idx to i64
    %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i64 %idx64
    store i32 %result, ptr addrspace(1) %ptr, align 4
    br label %exit

exit:
    ret void
}
