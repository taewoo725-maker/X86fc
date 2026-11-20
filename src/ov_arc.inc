; 만든이: KimTaeWoo
; X86FC 압축 파일 목록 보기 (오버레이 6) - SPDX-License-Identifier: GPL-3.0-or-later
; (0.12.47) 풀기는 오버레이 13(ov_arcx.inc)으로 나눔: 목록에서 Enter·INS 를 누르면 그 칸으로 건너가고, 끝나면 진입 1(ar_resume)·2(ar_back)로 돌아옴.
; 공통 루틴(머리 해석·표시 비트)은 arccom.inc, 공통 정의는 arcdef.inc.
                include "arcdef.inc"
do_arc:         ld      hl,AV
                ld      de,AV+1
                ld      bc,AVN-1
                ld      (hl),0
                ldir
                ld      a,(RUNPATH)
                or      a
                jp      z,ar_nz
                ld      de,RUNPATH
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jp      nz,ar_nz                ; 못 열면 압축 파일로 보지 않음 (본체가 따로 알림)
                ld      a,b
                ld      (ar_h),a
                ld      hl,0
                ld      (ar_n),hl
                ld      de,0
                ld      bc,16
                call    ar_rd
                ld      a,(SECBUF)              ; --- 서명으로 형식 가려내기
                cp      'P'
                jr      nz,ar_t1
                ld      a,(SECBUF+1)
                cp      'K'
                jr      nz,ar_no
                ld      a,(SECBUF+2)
                cp      3
                jr      nz,ar_no
                ld      a,2
                jr      ar_fm
ar_t1:          cp      60h
                jr      nz,ar_t2
                ld      a,(SECBUF+1)
                cp      0EAh
                jr      nz,ar_no
                ld      a,3
                jr      ar_fm
ar_t2:          ld      a,(SECBUF+2)            ; LZH: [2] = '-', [6] = '-', [3] = 'l' 또는 'p'
                cp      '-'
                jr      nz,ar_no
                ld      a,(SECBUF+6)
                cp      '-'
                jr      nz,ar_no
                ld      a,(SECBUF+3)
                cp      'l'
                jr      z,ar_t3
                cp      'p'
                jr      nz,ar_no
ar_t3:          ld      a,1
ar_fm:          ld      (ar_fmt),a
                ld      hl,AR_MARK
                ld      de,AR_MARK+1
                ld      bc,31
                ld      (hl),0
                ldir
                call    ar_scan
                call    ar_ui
                ld      a,(ar_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                xor     a                       ; Z = 처리함
                ret
ar_no:          ld      a,(ar_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
ar_nz:          or      1                       ; NZ = 압축 파일이 아님
                ret

; 지금 ar_pos 를 항목 표에 적음 → carry = 적었음, NC = 가득
ar_note:        ld      hl,(ar_n)
                ld      de,AR_MAX
                or      a
                sbc     hl,de
                ret     nc
                ld      hl,(ar_n)
                add     hl,hl
                add     hl,hl
                ld      de,AR_OFF
                add     hl,de
                ex      de,hl
                ld      hl,ar_pos
                ld      bc,4
                ldir
                ld      hl,(ar_n)
                inc     hl
                ld      (ar_n),hl
                scf
                ret

; ---- 목록 모으기
ar_scan:        ld      hl,0
                ld      (ar_pos),hl
                ld      (ar_pos+2),hl
                ld      a,(ar_fmt)
                cp      2
                jp      z,ar_zip
                cp      3
                jr      z,ar_arj
ar_lzh:         ld      bc,32                   ; LZH/PMA: 머리를 따라가며 (머리 크기 + 압축 크기)
                call    ar_rdp
                ld      de,22
                or      a
                sbc     hl,de
                ret     c
                ld      a,(SECBUF+2)
                cp      '-'
                ret     nz
                ld      a,(SECBUF+6)
                cp      '-'
                ret     nz
                ld      hl,(SECBUF)             ; 머리 크기: 수준 2 = 16비트, 수준 0·1 = 바이트 + 2
                ld      a,(SECBUF+20)
                cp      2
                jr      z,al_l2
                ld      h,0
                ld      a,l
                or      a
                ret     z                       ; 0 = 끝
                inc     hl
                inc     hl
al_l2:          ld      a,h
                or      l
                ret     z
                push    hl
                call    ar_note
                pop     hl
                ret     nc
                ld      de,0
                call    ar_add                  ; + 머리
                ld      hl,(SECBUF+7)
                ld      de,(SECBUF+9)
                call    ar_add                  ; + 압축 크기
                jr      ar_lzh
ar_arj:         ld      bc,40                   ; ARJ: 60 EA, 머리 크기(2), 본 머리, CRC(4), 확장 머리들, 압축 자료
                call    ar_rdp
                ld      de,34
                or      a
                sbc     hl,de
                ret     c
                ld      a,(SECBUF)
                cp      60h
                ret     nz
                ld      a,(SECBUF+1)
                cp      0EAh
                ret     nz
                ld      hl,(SECBUF+2)
                ld      a,h
                or      l
                ret     z
                ld      a,(SECBUF+10)           ; 파일 종류 2 = 아카이브 머리 (항목 아님)
                cp      2
                jr      z,aa_mh
                call    ar_note
                ret     nc
                ld      hl,(SECBUF+16)          ; 압축 크기를 나중에 더하려고 보관
                ld      (ar_cs),hl
                ld      hl,(SECBUF+18)
                ld      (ar_cs+2),hl
                jr      aa_nn
aa_mh:          ld      hl,0                    ; 아카이브 머리: 압축 자료 없음 (그 칸은 다른 뜻)
                ld      (ar_cs),hl
                ld      (ar_cs+2),hl
aa_nn:
                ld      hl,(SECBUF+2)
                ld      de,8                    ; 60 EA + 크기(2) + 머리 + CRC(4) = 8 + 머리 크기
                add     hl,de
                ld      de,0
                call    ar_add
aa_x:           ld      bc,2                    ; 확장 머리: 크기(2), 0 이면 끝, 아니면 크기 + 6
                call    ar_rdp
                ld      a,h
                or      l
                ret     z
                ld      hl,(SECBUF)
                ld      de,0
                ld      a,h
                or      l
                jr      z,aa_xe
                ld      bc,6
                add     hl,bc
                call    ar_add
                jr      aa_x
aa_xe:          ld      hl,2
                call    ar_add
                ld      hl,(ar_cs)
                ld      de,(ar_cs+2)
                call    ar_add
                jr      ar_arj
ar_zip:         ld      a,(ar_h)                ; ZIP: 끝에서 중앙 디렉터리 끝 표식 (50 4B 05 06) 찾기
                ld      b,a
                ld      de,0
                ld      hl,0
                ld      a,2
                ld      c,F_SEEK
                call    BDOS                    ; DE:HL = 파일 크기
                ld      bc,512
                ld      a,d
                or      e
                jr      nz,az_r
                push    hl
                or      a
                sbc     hl,bc
                pop     hl
                jr      nc,az_r
                ld      b,h
                ld      c,l
az_r:           push    bc
                or      a
                sbc     hl,bc
                jr      nc,az_h
                dec     de
az_h:           call    ar_rd
                pop     bc
                ld      hl,SECBUF
                add     hl,bc
                ld      de,-22
                add     hl,de
az_f:           ld      de,SECBUF
                or      a
                sbc     hl,de
                ret     c
                add     hl,de
                ld      a,(hl)
                cp      50h
                jr      nz,az_n
                inc     hl
                ld      a,(hl)
                cp      4Bh
                jr      nz,az_n1
                inc     hl
                ld      a,(hl)
                cp      5
                jr      nz,az_n2
                inc     hl
                ld      a,(hl)
                cp      6
                jr      nz,az_n3
                dec     hl
                dec     hl
                dec     hl                      ; HL = 표식 시작
                jr      az_ok
az_n3:          dec     hl
az_n2:          dec     hl
az_n1:          dec     hl
az_n:           dec     hl
                jr      az_f
az_ok:          ld      de,10
                add     hl,de
                ld      c,(hl)                  ; 항목 수
                inc     hl
                ld      b,(hl)
                inc     hl
                inc     hl
                inc     hl
                inc     hl
                inc     hl
                ld      e,(hl)                  ; 중앙 디렉터리 위치 (+16)
                inc     hl
                ld      d,(hl)
                inc     hl
                ld      (ar_pos),de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      (ar_pos+2),de
                ld      (ar_zn),bc
az_l:           ld      hl,(ar_zn)
                ld      a,h
                or      l
                ret     z
                dec     hl
                ld      (ar_zn),hl
                ld      bc,46
                call    ar_rdp
                ld      de,46
                or      a
                sbc     hl,de
                ret     c
                ld      a,(SECBUF)
                cp      50h
                ret     nz
                ld      a,(SECBUF+2)
                cp      1
                ret     nz
                call    ar_note
                ret     nc
                ld      hl,(SECBUF+28)          ; 이름 + 덧붙임 + 설명 길이 + 46
                ld      de,(SECBUF+30)
                add     hl,de
                ld      de,(SECBUF+32)
                add     hl,de
                ld      de,46
                add     hl,de
                ld      de,0
                call    ar_add
                jr      az_l

; ---- 화면
ar_ui:          call    clear_screen
                ld      hl,0
                ld      (ar_top),hl
                ld      (ar_cur),hl
                call    ar_head
                call    ar_all
ar_key:         call    wait_key
                cp      1Bh
                ret     z
                cp      K_F10
                ret     z
                cp      K_DOWN
                jr      z,ak_dn
                cp      K_UP
                jr      z,ak_up
                cp      K_RIGHT
                jr      z,ak_pd
                cp      K_LEFT
                jr      z,ak_pu
                cp      K_HOME
                jr      z,ak_hm
                cp      ' '
                jp      z,ak_mark
                cp      01h
                jp      z,ak_alm
                cp      'a'
                jp      z,ak_alm
                cp      'A'
                jp      z,ak_alm
                cp      0Dh
                jp      z,ar_ext
                cp      K_INS
                jp      z,ar_ext
                jr      ar_key
ak_dn:          ld      hl,(ar_cur)
                inc     hl
                jr      ak_go
ak_up:          ld      hl,(ar_cur)
                ld      a,h
                or      l
                jr      z,ar_key
                dec     hl
                jr      ak_go
ak_pd:          ld      hl,(ar_cur)
                ld      de,AR_ROWS
                add     hl,de
                jr      ak_go
ak_pu:          ld      hl,(ar_cur)
                ld      de,AR_ROWS
                or      a
                sbc     hl,de
                jr      nc,ak_go
ak_hm:          ld      hl,0
                jr      ak_go
ak_go:          ld      de,(ar_n)               ; 범위 0 .. n-1
                ld      a,d
                or      e
                jr      z,ar_key
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,ak_ok
                ex      de,hl
                dec     hl
ak_ok:          ld      de,(ar_cur)
                or      a
                sbc     hl,de                   ; (HL - 옛 커서) 0 이면 그대로
                add     hl,de
                jp      z,ar_key
                ld      (ar_cur),hl
                ld      (ar_old),de
                ld      hl,(ar_cur)             ; 화면 밖이면 맨 위를 옮겨 전체를 다시 그림, 안이면 두 줄만
                ld      de,(ar_top)
                or      a
                sbc     hl,de
                jr      c,ak_up2
                ld      de,AR_ROWS
                or      a
                sbc     hl,de
                jr      c,ak_two
                ld      hl,(ar_cur)
                ld      de,AR_ROWS-1
                or      a
                sbc     hl,de
                jr      ak_top
ak_up2:         ld      hl,(ar_cur)
ak_top:         ld      (ar_top),hl
                call    ar_all
                jp      ar_key
ak_two:         ld      hl,(ar_old)             ; 옛 줄과 새 줄만
                ld      de,(ar_top)
                or      a
                sbc     hl,de
                ld      a,l
                call    ar_row
                ld      hl,(ar_cur)
                ld      de,(ar_top)
                or      a
                sbc     hl,de
                ld      a,l
                call    ar_row
                jp      ar_key

; 목록 전체 (20 줄)
ar_all:         xor     a
aa_l:           push    af
                call    ar_row
                pop     af
                inc     a
                cp      AR_ROWS
                jr      c,aa_l
                ret

; A = 목록 줄 번호(0~19) 하나를 그림
ar_row:         ld      (ar_sr),a
                ld      e,a
                ld      d,0
                ld      hl,(ar_top)
                add     hl,de
                ld      (ar_ix),hl
                ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      hl,(ar_ix)
                ld      de,(ar_n)
                or      a
                sbc     hl,de
                jr      nc,arw_w                ; 항목 번호 >= 개수: 빈 줄
                ld      de,(ar_ix)
                call    ar_rec
                ld      hl,AR_REC+R_NAME
                ld      de,hbuf+2
                ld      b,30
arw_n:          ld      a,(hl)
                or      a
                jr      z,arw_s
                ld      (de),a
                inc     hl
                inc     de
                djnz    arw_n
arw_s:          ld      hl,AR_REC+R_OSZ
                ld      de,hbuf+32
                call    dec32
                ld      hl,AR_REC+R_CSZ
                ld      de,hbuf+43
                call    dec32
                ld      hl,(AR_REC+R_DATE)
                ld      a,h
                or      l
                jr      z,arw_m
                ld      de,hbuf+54
                call    ar_date
arw_m:          ld      hl,AR_REC+R_METH
                ld      de,hbuf+63
                call    copy_z
                ld      hl,(ar_ix)              ; 표시 '*'
                call    mk_addr
                and     (hl)
                jr      z,arw_c
                ld      a,'*'
                ld      (hbuf),a
arw_c:          ld      hl,(ar_ix)
                ld      de,(ar_cur)
                or      a
                sbc     hl,de
                jr      nz,arw_w
                ld      a,'>'                   ; 커서 줄
                ld      (hbuf+1),a
                ld      a,9
                ld      (s7_pair),a
arw_w:          ld      a,(ar_sr)
                add     a,2
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                xor     a
                ld      (s7_pair),a
                ret
; HL = DOS 날짜, DE = 쓸 곳 → "yy-mm-dd"
ar_date:        ld      b,h
                ld      c,l
                ld      a,b
                srl     a
                add     a,80
ad_y:           cp      100
                jr      c,ad_y2
                sub     100
                jr      ad_y
ad_y2:          call    put2
                ld      a,'-'
                ld      (de),a
                inc     de
                ld      a,c
                rlca
                rlca
                rlca
                and     7
                ld      l,a
                ld      a,b
                and     1
                rlca
                rlca
                rlca
                or      l
                call    put2
                ld      a,'-'
                ld      (de),a
                inc     de
                ld      a,c
                and     1Fh
                jp      put2

; ---- 머리·안내 줄
ar_head:        ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      de,hbuf
                ld      a,(ar_fmt)
                ld      hl,ar_tlzh
                cp      2
                jr      c,ah_f
                ld      hl,ar_tzip
                jr      z,ah_f
                ld      hl,ar_tarj
ah_f:           call    copy_z
                ld      hl,RUNPATH
                ld      b,44
ah_p:           ld      a,(hl)
                or      a
                jr      z,ah_n
                ld      (de),a
                inc     hl
                inc     de
                djnz    ah_p
ah_n:           ld      a,' '
                ld      (de),a
                inc     de
                ld      (de),a
                inc     de
                ld      hl,(ar_n)
                call    put_dec16
                ld      hl,ar_tfiles
                call    copy_z
                xor     a
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      hl,ar_tcol
                ld      a,1
                call    ar_ln
                ld      hl,ar_thint
                ld      a,23
ar_ln:          push    af
                push    hl
                ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                pop     hl
                ld      de,hbuf
                call    copy_z
                pop     af
                call    row_addr
                ld      de,hbuf
                ld      b,80
                jp      scr_write
ar_tlzh:        db      "LZH: ",0
ar_tzip:        db      "ZIP: ",0
ar_tarj:        db      "ARJ: ",0
ar_tfiles:      db      " files",0
ar_tcol:        db      "   Name                           Original    Packed Date     Meth",0
ar_thint:       db      " Up/Dn  Space mark  A all  Enter extract to...  ESC close",0


; ======================================================================
; (0.12.38) 표시 · 풀기 (LZH).  풀기는 매퍼 쪽 하나에 ov_lzh.inc(오버레이 7)를 읽어 와서 실행
; ======================================================================
; Space: 커서 항목 표시 토글 뒤 한 줄 아래로
ak_mark:        ld      hl,(ar_cur)
                call    mk_addr
                xor     (hl)
                ld      (hl),a
                ld      hl,(ar_cur)
                ld      de,(ar_top)
                or      a
                sbc     hl,de
                ld      a,l
                call    ar_row
                jp      ak_dn
; Ctrl+A / A: 하나라도 안 고른 게 있으면 모두 고름, 아니면 모두 해제
ak_alm:         call    ar_cnt
                ld      de,(ar_n)
                or      a
                sbc     hl,de
                ld      a,0FFh
                jr      nz,ak_af
                xor     a
ak_af:          ld      hl,AR_MARK
                ld      b,32
ak_fl:          ld      (hl),a
                inc     hl
                djnz    ak_fl
                call    ar_all
                jp      ar_key


; ---- 풀기(오버레이 13)로 건너감 / 거기서 돌아옴
ar_ext:         pop     hl                      ; ov_ret 버리고 (ov_go 가 다시 쌓음)
                ld      a,13
                ld      b,0
                jp      ov_go
ar_resume:      ld      ix,(ar_pix)             ; (진입 1) 풀기 뒤: 반대 패널을 다시 읽고 목록을 다시 그림
                call    panel_load
                call    cur_ix                  ; (0.12.56) 활성 패널(압축 파일이 있는 쪽)도 다시 읽음: 같은 폴더에 풀면 새 파일이 바로 보이게 (커서는 압축 파일 이름으로)
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    disp_name
                ld      hl,nmbuf
                ld      de,upname
                call    strcpy_nul
                call    cur_ix
                push    ix
                call    panel_load
                pop     ix
                call    find_exact
ar_back:        call    clear_screen            ; (진입 2) 목록 다시 그림
                call    ar_head
                call    ar_all
                jp      ar_key
                include "arccom.inc"
