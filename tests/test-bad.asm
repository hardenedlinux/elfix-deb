global badness:function

SECTION .text

align 16
badness:

    push RBX
    push RBP

    pop RBP
    pop RBX
    ret

end
