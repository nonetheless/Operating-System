;数列显示
data     segment
         msg0    db 'pleace input a number.$'
         color   db 01h,04h,07h
         colore  label byte
         x1      dw 1
         x2      dw 1
         fn      dw 22 DUP(?)
         num     dw ?
data     ends
;
code     segment
         assume   cs:code, ds:data
start:
         mov   ax,data
         mov   ds,ax
         mov   dx,offset  msg0
         mov   ah,9h
         int   21h                          ;显示请输入
         mov   ah,1h
         int   21h
         mov   bl,al
         mov   ah,1h
         int   21h
         cmp   al,0dh
         jz    ONEL
         mov   bh,bl
         mov   bl,al
         call  subr
         call  newline
         jmp   NEXT1
ONEL:
         mov   al,bl
         and   al,0fh
NEXT1:
         xor   cx,cx
         xor   ah,ah
         mov   cl,al
         mov   num,ax
         cmp   cx,2
         jb    DON
         cmp   cx,2
         jz    DON
         sub   cx,2
         mov   bx,0
         push  x1
         push  x2
LOOP1:
         mov   ax,x1
         add   ax,x2
         mov   fn[bx],ax
         push  x2
         pop   x1
         mov   x2,ax
         add   bx,2
         loop  LOOP1
         pop   x2
         pop   x1
DON:     lea   si,x1
         lea   di,color
         mov   cx,num
LOOP2:
         call  show
         inc   di
         cmp   di,offset colore
         jnz   NTE
         mov   di,offset color
NTE:
         add   si,2
         mov   ah,02h
         mov   dl,20h
         int   21h
         cmp   cx,0
         sub   cx,1
         jne   loop2
         mov   ax,4c00h
         int   21h
show:
         push  cx
         mov   ax,[si]
         mov   bx,10 
         
         xor   cx,cx
THEN: 
         mov   dx,0
         div   bx
         push  dx
         inc   cx
         cmp   ax,0
         je    OUTP
         jmp   THEN
OUTP:
         pop   dx
         add   dl,30h
         mov   al,dl
         mov   bl,[di]
         call  output
         loop  OUTP
         pop   cx
         ret

output:
         push bx
         push dx
         push cx
         mov  cx,1
         mov  bh,0
         mov  ah,9
         int  10h
         mov  ah,3
         int  10h
         inc  dl
         mov  ah,2
         int  10h
         pop  cx
         pop  dx
         pop  bx
         ret
;子程序
newline      proc
             push  ax
             push  dx
             mov   dl,0dh
             mov   ah,2
             int   21h
             mov   dl,0ah
             mov   ah,2
             int   21h
             pop   dx
             pop   ax
             ret
newline      endp
;
subr         proc
             mov   al,bh
             and   al,0fh
             mov   ah,10
             mul   ah
             mov   ah,bl
             and   ah,0fh
             add   al,ah
             ret
subr         endp
CODE ENDS
	END START
        
         

         