; GREET.COM - minimal CP/M 2.2 program: print a string via BDOS function 9.
; Assembled with pasmo (org 0100h = the CP/M Transient Program Area).
; MIT licensed, part of runcpm-wasm.
        org     0100h
        ld      de,msg          ; DE -> '$'-terminated string
        ld      c,9             ; BDOS function 9 = print string
        call    0005h           ; BDOS entry
        ret                     ; back to the CCP
msg:    db      'RunCPM in a browser tab. Clean-room CCP, no DR code.',13,10,'$'
