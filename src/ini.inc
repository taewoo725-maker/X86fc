; 만든이: KimTaeWoo
; X86FC INI 엔진 (0.12.14) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; X86FC.INI 한 개에 설정·연결·메뉴·Quick-Dir·명령줄 기록을 모두 둔다 (옛 SET·MNU·HIS·QD 파일은 첫 실행 때 옮김).
; 읽기: 파일 전체를 512바이트씩 스트림으로 읽어 줄 하나씩 ILBUF 에 줄 (il_next).  파일 크기 한도 없음.
;   줄이 254글자보다 길면 254글자씩 끊어 주고 il_cont/il_tr 로 알려 줌 (쓸 때 그대로 이어 붙임).
; 쓰기: ini_wr = 임시 파일(X86FC.TMP)에 옛 INI 를 줄 단위로 복사하되 "지울 줄" 판별 루틴이 맞는 줄은 빼고,
;   끝에 새 줄 만들기 루틴이 쓰는 줄을 붙임. 다른 줄·주석·순서는 그대로. 성공하면 INI -> OLD -> (TMP -> INI) -> OLD 지움
;   (중간에 실패하면 옛 INI 는 그대로).  줄 끝은 CR LF 로 맞춤.
; 키: SCREEN, SORT, MENU, QDIR, HIST (줄 앞의 키워드, 대소문자 무시, 키워드 뒤에 공백이나 = ).
; ----------------------------------------------------------------------

; HL = 확장자 3글자 -> CMDBUF = X86FC.COM 폴더의 X86FC.<확장자>,  DE = CMDBUF
xpath:          push    hl
                ld      hl,SELFPATH
                ld      de,CMDBUF
                call    copy_z
                xor     a
                ld      (de),a
                dec     de
                dec     de
                dec     de
                pop     hl
                ld      bc,3
                ldir
                ld      de,CMDBUF
                ret
ini_path:       ld      hl,x_ini
                jr      xpath
x_ini:          db      "INI"
x_tmp:          db      "TMP"
x_old:          db      "OLD"
x_ovl:          db      "OVL"
x_qd:           db      "QD",0
x_set:          db      "SET"
x_mnu:          db      "MNU"
x_his:          db      "HIS"
nm_old:         db      "X86FC.OLD",0
nm_ini:         db      "X86FC.INI",0

; ---- 읽기 ----
il_open:        call    ini_path
il_openf:       ld      de,CMDBUF               ; (경로가 CMDBUF 에 있음)
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (ir_h),a
                ld      hl,0
                ld      (ir_n),hl
                xor     a
                ld      (ir_eof),a
                ld      (il_tr),a
                ret                             ; Z = 열림
il_close:       ld      a,(ir_h)
                ld      b,a
                ld      c,F_CLOSE
                jp      BDOS
; 글자 하나 -> A (NC),  끝이면 C
ir_get:         ld      hl,(ir_n)
                ld      a,h
                or      l
                jr      z,ig_f
ig_g:           dec     hl
                ld      (ir_n),hl
                ld      hl,(ir_p)
                ld      a,(hl)
                inc     hl
                ld      (ir_p),hl
                or      a
                ret
ig_f:           ld      a,(ir_eof)
                or      a
                jr      nz,ig_e
                ld      a,(ir_h)
                ld      b,a
                ld      de,IRBUF
                ld      hl,512
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,ig_s
                or      a
                jr      z,ig_k
                ld      hl,0
ig_s:           ld      a,1
                ld      (ir_eof),a
ig_k:           ld      (ir_n),hl
                ld      hl,IRBUF
                ld      (ir_p),hl
                ld      hl,(ir_n)
                ld      a,h
                or      l
                jr      nz,ig_g
ig_e:           scf
                ret
; 다음 줄 -> ILBUF (끝 0).  Z = 더 없음.  il_cont = 지난 덩어리가 길어서 이어진 것, il_tr = 이 덩어리가 중간에 끊김
il_next:        ld      a,(il_tr)
                ld      (il_cont),a
                xor     a
                ld      (il_tr),a
                ld      (il_any),a
                ld      hl,ILBUF
                ld      (il_dp),hl
iln_l:          call    ir_get
                jr      c,iln_f
                cp      10
                jr      z,iln_nl
                cp      13
                jr      z,iln_l
                ld      c,a
                ld      hl,(il_dp)
                ld      de,ILBUF+254
                or      a
                sbc     hl,de
                jr      nc,iln_full
                add     hl,de
                ld      (hl),c
                inc     hl
                ld      (il_dp),hl
                ld      a,1
                ld      (il_any),a
                jr      iln_l
iln_full:       ld      hl,(ir_p)
                dec     hl
                ld      (ir_p),hl
                ld      hl,(ir_n)
                inc     hl
                ld      (ir_n),hl
                ld      a,1
                ld      (il_tr),a
                jr      iln_f
iln_nl:         ld      a,1
                ld      (il_any),a
iln_f:          ld      hl,(il_dp)
                ld      (hl),0
                ld      a,(il_any)
                or      a
                ret
; ILBUF 의 줄이 키워드 DE (소문자, 0 끝)로 시작하나 -> Z = 맞음, HL = 키워드 뒤
il_key:         ld      hl,ILBUF
ik_s:           ld      a,(hl)
                cp      ' '
                jr      z,ik_n
                cp      9
                jr      nz,ik_c
ik_n:           inc     hl
                jr      ik_s
ik_c:           ld      a,(de)
                or      a
                jr      z,ik_e
                ld      b,a
                ld      a,(hl)
                or      20h
                cp      b
                ret     nz
                inc     hl
                inc     de
                jr      ik_c
ik_e:           ld      a,(hl)
                cp      ' '
                ret     z
                cp      '='
                ret     z
                cp      9
                ret
; HL = 키워드 뒤 -> HL = 값 처음 (공백·= 건너뜀)
il_val:         ld      a,(hl)
                cp      ' '
                jr      z,iv_n
                cp      '='
                jr      z,iv_n
                cp      9
                ret     nz
iv_n:           inc     hl
                jr      il_val

; ---- 쓰기 ----
; HL = "지울 줄" 판별 루틴 (들어갈 때 HL = ILBUF, Z = 지움),  DE = 새 줄 만들기 루틴 (iw_put / iw_blk 로 씀)
; 결과 Z = 성공
ini_wr:         ld      (iw_mt),hl
                ld      (iw_gn),de
                xor     a
                ld      (iw_err),a
                ld      (iw_drop),a
                ld      (iw_had),a
                ld      hl,x_tmp
                call    xpath
                xor     a
                ld      b,0
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,iw_cr
                or      1
                ret
iw_cr:          ld      a,b
                ld      (iw_h),a
                call    il_open
                jr      nz,iw_gen
                ld      a,1
                ld      (iw_had),a
iwl:            call    il_next
                jr      z,iw_srcend
                ld      a,(il_cont)
                or      a
                jr      nz,iw_cont
                call    iw_callmt
                jr      nz,iw_keep
                ld      a,(il_tr)
                ld      (iw_drop),a
                jr      iwl
iw_cont:        ld      a,(iw_drop)
                or      a
                jr      z,iw_keep
                ld      a,(il_tr)
                ld      (iw_drop),a
                jr      iwl
iw_keep:        xor     a
                ld      (iw_drop),a
                ld      hl,ILBUF
                call    iw_raw
                ld      a,(il_tr)
                or      a
                call    z,iw_crlf
                jr      iwl
iw_srcend:      call    il_close
iw_gen:         ld      hl,(iw_gn)
                call    iw_jp
                ld      a,(iw_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      a,(iw_err)
                or      a
                jr      nz,iw_bad
                ld      hl,x_old                ; 지난번 남은 OLD 는 지움
                call    xpath
                ld      c,F_DELETE
                call    BDOS
                ld      a,(iw_had)
                or      a
                jr      z,iw_nr
                ld      hl,x_ini                ; INI -> OLD
                call    xpath
                ld      hl,nm_old
                ld      c,F_RENAME
                call    BDOS
                or      a
                jr      nz,iw_bad
iw_nr:          ld      hl,x_tmp                ; TMP -> INI
                call    xpath
                ld      hl,nm_ini
                ld      c,F_RENAME
                call    BDOS
                or      a
                jr      z,iw_ok
                ld      a,(iw_had)              ; 실패: OLD 를 INI 로 되돌림
                or      a
                jr      z,iw_bad
                ld      hl,x_old
                call    xpath
                ld      hl,nm_ini
                ld      c,F_RENAME
                call    BDOS
                jr      iw_bad
iw_ok:          ld      hl,x_old
                call    xpath
                ld      c,F_DELETE
                call    BDOS
                xor     a
                ret
iw_bad:         ld      hl,x_tmp
                call    xpath
                ld      c,F_DELETE
                call    BDOS
                or      1
                ret
iw_callmt:      ld      hl,(iw_mt)
                push    hl
                ld      hl,ILBUF
                ret
iw_jp:          jp      (hl)
; 키워드 판별: 줄이 키워드 (iw_key)로 시작하면 지움
iw_mkey:        ld      de,(iw_key)
                jp      il_key
; HL = 글 (0 끝) -> 쓰기 (줄바꿈 없이) / iw_put = 글 + CR LF / iw_blk: DE = 주소, HL = 길이 / iw_crlf
iw_raw:         push    hl
                call    strlen
                pop     de
                ld      l,a
                ld      h,0
iw_blk:         ld      a,h
                or      l
                ret     z
                ld      a,(iw_h)
                ld      b,a
                ld      c,F_WRITE
                call    BDOS
                or      a
                ret     z
                ld      a,1
                ld      (iw_err),a
                ret
iw_crlf:        ld      de,iw_nl
                ld      hl,2
                jr      iw_blk
iw_put:         call    iw_raw
                jr      iw_crlf
iw_nl:          db      13,10
ir_h:           db      0
ir_eof:         db      0
ir_n:           dw      0
ir_p:           dw      0
il_tr:          db      0
il_cont:        db      0
il_any:         db      0
il_dp:          dw      0
iw_h:           db      0
iw_err:         db      0
iw_drop:        db      0
iw_had:         db      0
iw_mt:          dw      0
iw_gn:          dw      0
iw_key:         dw      0
k_qdir:         db      "qdir",0
k_hist:         db      "hist",0
k_menu:         db      "menu",0
txt_qdk:        db      "QDIR = ",0
txt_hik:        db      "HIST = ",0

; ---- 옛 파일 이행 (첫 실행 때 한 번: SET·MNU·QD·HIS 가 있으면 INI 로 옮기고 지움) ----
; 하나라도 있으면 오버레이 5 의 mig_old 를 부름
mig_chk:        ld      hl,mg_ext
                ld      b,4
mc_l:           push    bc
                push    hl
                call    xpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      nz,mc_n
                ld      c,F_CLOSE
                call    BDOS
                pop     hl
                pop     bc
                ld      a,5
                ld      b,1
                jp      ov_go
mc_n:           pop     hl
                ld      de,3
                add     hl,de
                pop     bc
                djnz    mc_l
                ret
mg_ext:         db      "SET","MNU","QD",0,"HIS"
