; 만든이: KimTaeWoo
; X86FC 사용자 메뉴 - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; Ctrl+G = 사용자 메뉴 팝업.  X86FC.INI 의 "MENU 제목 = 명령" 줄들을 보여 주고, 0-9/A-J 로 고르면 실행.
;   명령이 "CD 경로" 이면 활성 패널을 그 폴더로, 아니면 아래 줄 명령줄('/')과 같은 방식으로 실행.
;   %s = 커서 항목 이름, %f = 확장자 뺀 이름.   (M 의 <FKEY> 사용자 메뉴에서 얻은 아이디어)
; ----------------------------------------------------------------------
UM_MAX          equ     20

ml_umenu:       call    typeclr
                call    um_load
                xor     a
                ld      (um_sel),a              ; (0.12.10) 처음엔 첫 항목 선택
um_show:        ld      a,(um_n)               ; (선택 번호를 목록 길이 안으로)
                ld      b,a
                ld      a,(um_sel)
                cp      b
                jr      c,um_sk
                ld      a,b
                or      a
                jr      z,um_sk
                dec     a
um_sk:          ld      (um_sel),a
                call    draw_all
                ld      a,(um_n)
                or      a
                jr      nz,um_h1
                inc     a                       ; 비었으면 "없음" 한 줄 (INS 로 추가)
um_h1:          ld      (qd_nl),a
                add     a,3
                ld      b,a
                ld      a,24
                sub     b
                srl     a
                ld      (qd_y0),a
                ld      hl,txt_umt
                xor     a
                ld      b,a
                call    qd_prow
                xor     a
                ld      (qd_i),a
um_dl:          ld      a,(qd_i)
                ld      hl,um_n
                cp      (hl)
                jr      nc,um_dn
                call    um_ln
                ld      a,(qd_i)
                inc     a
                ld      (qd_i),a
                jr      um_dl
; A = 번호: 그 항목 줄 하나를 그림 (선택이면 '>' 표시)
um_ln:          ld      (qd_i),a
                ld      hl,um_sel
                cp      (hl)
                ld      a,' '
                jr      nz,um_lm
                ld      a,'>'
um_lm:          ld      de,errbuf
                ld      (de),a
                inc     de
                ld      a,(qd_i)
                cp      10
                jr      c,um_l1
                add     a,'A'-10
                jr      um_l2
um_l1:          add     a,'0'
um_l2:          ld      (de),a
                inc     de
                ld      a,' '
                ld      (de),a
                inc     de
                ld      a,(qd_i)
                call    um_ent                  ; HL = 항목 (제목, 명령)
                push    hl
                ld      c,(hl)
                inc     hl
                ld      h,(hl)
                ld      l,c                     ; HL = 제목
                ld      b,12
um_tc:          ld      a,(hl)
                or      a
                jr      z,um_tp
                ld      (de),a
                inc     hl
                inc     de
                djnz    um_tc
                jr      um_tg
um_tp:          ld      a,' '
um_tp1:         ld      (de),a
                inc     de
                djnz    um_tp1
um_tg:          ld      a,' '
                ld      (de),a
                inc     de
                pop     hl
                inc     hl
                inc     hl
                ld      c,(hl)
                inc     hl
                ld      h,(hl)
                ld      l,c                     ; HL = 명령
                ld      b,46
um_cc:          ld      a,(hl)
                or      a
                jr      z,um_ce
                ld      (de),a
                inc     hl
                inc     de
                djnz    um_cc
um_ce:          xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      a,(qd_i)
                inc     a
                ld      b,1
                jp      qd_prow
um_dn:          ld      a,(um_n)
                or      a
                jr      nz,um_dh
                ld      hl,txt_umnone
                ld      a,1
                ld      b,1
                call    qd_prow
um_dh:          ld      hl,txt_umh
                call    qd_hint
                ld      a,(qd_nl)
                add     a,2
                ld      b,2
                ld      hl,0
                call    qd_prow
um_k:           call    wait_key
                cp      1Bh
                jr      z,um_esc
                cp      K_INS
                jp      z,um_ins
                cp      K_DEL
                jp      z,um_del
                cp      K_F2
                jp      z,um_ed
                cp      K_DOWN
                jr      z,um_dn1
                cp      K_UP
                jr      z,um_up1
                cp      0Dh
                jr      z,um_rt
                call    qd_idx
                jr      c,um_k
                ld      b,a
                ld      a,(um_n)
                ld      c,a
                ld      a,b
                cp      c
                jr      nc,um_k
um_go:          call    um_ent
                inc     hl
                inc     hl
                ld      c,(hl)
                inc     hl
                ld      h,(hl)
                ld      l,c                     ; HL = 고른 명령
                jp      um_run
um_dn1:         ld      a,(um_sel)             ; (0.12.10) ↓: 두 줄(예전·새 선택)만 다시 그림
                inc     a
                ld      hl,um_n
                cp      (hl)
                jp      nc,um_k
                jr      um_mv
um_up1:         ld      a,(um_sel)
                or      a
                jp      z,um_k
                dec     a
um_mv:          ld      b,a
                ld      a,(um_sel)
                ld      c,a
                ld      a,b
                ld      (um_sel),a
                push    bc
                ld      a,c
                call    um_ln
                pop     bc
                ld      a,b
                call    um_ln
                jp      um_k
um_rt:          ld      a,(um_n)               ; RETURN: 고른 항목 실행
                or      a
                jp      z,um_k
                ld      a,(um_sel)
                jr      um_go
um_sel:         db      0
um_esc:         call    draw_all
                jp      ml_next
; A = 번호 → HL = 항목 주소 (4 바이트: 제목 번지, 명령 번지)
um_ent:         ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                ld      bc,um_tab
                add     hl,bc
                ret

; HL = 명령.  "CD 경로" 면 폴더 이동, 아니면 명령줄처럼 실행
um_run:         ld      a,(hl)
                or      20h
                cp      'c'
                jr      nz,um_cmd
                inc     hl
                ld      a,(hl)
                or      20h
                cp      'd'
                jr      nz,um_cmd0
                inc     hl
                ld      a,(hl)
                cp      ' '
                jr      nz,um_cmd00
um_cd1:         inc     hl
                ld      a,(hl)
                cp      ' '
                jr      z,um_cd1
                push    hl                      ; --- 폴더 이동
                call    cur_ix
                call    path_save
                push    ix
                pop     de
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                pop     hl
                ld      b,63
um_cdc:         ld      a,(hl)
                ld      (de),a
                or      a
                jr      z,um_cdd
                inc     hl
                inc     de
                djnz    um_cdc
                xor     a
                ld      (de),a
um_cdd:         call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                jr      z,um_end
                call    explain_err
                call    path_restore
                call    panel_load
um_end:         call    draw_all
                jp      ml_next
um_cmd00:       dec     hl
um_cmd0:        dec     hl
um_cmd:         call    um_expand               ; HL = 명령 틀 → inbuf (%s %f 펼침)
                xor     a
                ld      (hook_sh),a
                ld      (cl_tmp),a
                call    dc_make                 ; 성공하면 돌아오지 않음 (적재기로 감)
                call    draw_all
                jp      ml_next
; HL = 명령 틀 → inbuf.  %s = 커서 항목 이름, %f = 확장자 뺀 이름
um_expand:      push    hl
                xor     a
                ld      (nmbuf),a
                call    cur_ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                jr      z,ux_g
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    disp_name
ux_g:           pop     hl
                ld      de,inbuf
                ld      b,100
ux_l:           ld      a,(hl)
                or      a
                jr      z,ux_e
                cp      '%'
                jr      nz,ux_lit
                inc     hl
                ld      a,(hl)
                or      20h
                cp      's'
                jr      z,ux_s
                cp      'f'
                jr      nz,ux_next
                push    hl
                ld      hl,nmbuf
ux_f:           ld      a,(hl)
                or      a
                jr      z,ux_fd
                cp      '.'
                jr      z,ux_fd
                ld      (de),a
                inc     hl
                inc     de
                djnz    ux_f
ux_fd:          pop     hl
                jr      ux_next
ux_s:           push    hl
                ld      hl,nmbuf
ux_sc:          ld      a,(hl)
                or      a
                jr      z,ux_sd
                ld      (de),a
                inc     hl
                inc     de
                djnz    ux_sc
ux_sd:          pop     hl
ux_next:        inc     hl
                ld      a,b
                or      a
                jr      nz,ux_l
                jr      ux_e
ux_lit:         ld      (de),a
                inc     de
                inc     hl
                djnz    ux_l
ux_e:           xor     a
                ld      (de),a
                ld      a,100
                sub     b
                ld      (in_len),a
                ret

; INI 를 읽어 "MENU 제목 = 명령" 줄을 um_tab(제목 번지, 명령 번지) 에 모음.  CPBUF 안을 직접 고침(끝 표시 0)
; X86FC.MNU 가 있으면 거기서, 없으면 예전 방식(INI 의 MENU 줄)으로 읽고 MNU 로 옮겨 둠
um_load:        xor     a                       ; (0.12.14) INI 의 MENU 줄들을 CPBUF 에 모아 아래 ul_ln 이 um_tab 으로 정리
                ld      (um_n),a
                call    um_rdmenu               ; HL = 길이
                ld      de,CPBUF
                add     hl,de
                ld      (hl),0
                ld      hl,CPBUF
                jr      ul_ln
; INI 의 "MENU ..." 줄들을 CPBUF 에 "줄 CR LF" 로 이어 붙임 -> HL = 글 길이
um_rdmenu:      ld      hl,CPBUF
                ld      (um_dp),hl
                call    il_open
                jr      nz,urd_e
urd_l:          call    il_next
                jr      z,urd_c
                ld      de,k_menu
                call    il_key
                jr      nz,urd_l
                ld      hl,(um_dp)
                ld      de,CPBUF+1600
                or      a
                sbc     hl,de
                jr      nc,urd_c
                ld      hl,ILBUF
                ld      de,(um_dp)
                call    copy_z
                ld      a,13
                ld      (de),a
                inc     de
                ld      a,10
                ld      (de),a
                inc     de
                ld      (um_dp),de
                jr      urd_l
urd_c:          call    il_close
urd_e:          ld      hl,(um_dp)
                ld      de,-CPBUF
                add     hl,de
                ret
um_dp:          dw      0
um_mi:          db      0
um_ok:          db      0
ul_ln:          ld      a,(hl)
                or      a
                ret     z
                cp      21h
                jr      nc,ul_go
                inc     hl
                jr      ul_ln
ul_go:          push    hl
                ld      de,um_kw
                call    s7i_kw
                jr      z,ul_m
                pop     hl
ul_sk:          ld      a,(hl)
                or      a
                ret     z
                inc     hl
                cp      10
                jr      nz,ul_sk
                jr      ul_ln
ul_m:           pop     de                      ; (키워드 뒤) 공백 필수
                ld      a,(hl)
                cp      ' '
                jr      nz,ul_sk
ul_s1:          inc     hl
                ld      a,(hl)
                cp      ' '
                jr      z,ul_s1
                ld      (um_t),hl               ; 제목 시작
ul_e:           ld      a,(hl)
                cp      '='
                jr      z,ul_eq
                or      a
                ret     z
                cp      10
                jr      z,ul_ln1
                inc     hl
                jr      ul_e
ul_ln1:         inc     hl
                jr      ul_ln
ul_eq:          ld      (hl),0                  ; 제목 끝
                inc     hl
ul_s2:          ld      a,(hl)
                cp      ' '
                jr      nz,ul_c
                inc     hl
                jr      ul_s2
ul_c:           ld      (um_c),hl               ; 명령 시작
ul_ee:          ld      a,(hl)
                or      a
                jr      z,ul_add
                cp      13
                jr      z,ul_et
                cp      10
                jr      z,ul_et
                inc     hl
                jr      ul_ee
ul_et:          ld      (hl),0
                inc     hl
ul_add:         push    hl
                ld      a,(um_n)
                cp      UM_MAX
                jr      nc,ul_full
                call    um_ent
                ld      de,(um_t)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                inc     hl
                ld      de,(um_c)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                ld      hl,um_n
                inc     (hl)
ul_full:        pop     hl
                jp      ul_ln
um_kw:          db      "menu",0
um_n:           db      0
um_t:           dw      0
um_c:           dw      0
um_tab:         ds      UM_MAX*4
txt_umt:        db      " User menu ",0
txt_umh:        db      "Up/Dn+Enter 0-9/A-J run  INS add  F2 edit  DEL del  ESC close",0
txt_umnone:     db      "(empty - press INS to add a menu item)",0
txt_umtp:       db      "Menu title (up to 12 chars)",0
txt_umcp:       db      "Command  (CD path = go to folder, %s %f = cursor name)",0
txt_umdel:      db      "Delete which one? (press its key)",0
txt_umed:       db      "Edit which one? (press its key)",0
txt_umps:       db      " %s",0
um_pc:          ds      104
um_oc:          ds      104
txt_umm:        db      "MENU ",0
txt_ume:        db      " = ",0

; ---- 메뉴 항목 추가·고치기·지우기 (INI 의 MENU 줄을 직접 고침) ----
; INS: 프로그램을 고르면(파일 선택 팝업) 제목 = 파일 이름, 명령 = "경로 %s" (확인 입력에서 고칠 수 있음).
;      고르기 창의 맨 위 "직접 입력" 줄을 고르면 제목·명령을 직접 침.
um_ins:         ld      a,(um_n)
                cp      UM_MAX
                jp      nc,ml_umenu
                call    pick_file               ; A = 0 고름(pk_path) / 1 직접 입력 / 2 취소
                cp      2
                jp      z,ml_umenu
                or      a
                jr      nz,ui_man
                call    um_fp_ttl
                call    um_fp_cmd
                ld      de,um_pc
                jr      ui_cmd
ui_man:         ld      hl,txt_umtp
                ld      de,0
                call    um_inp
                jp      nz,ml_umenu
                call    um_tget
                ld      de,0
ui_cmd:         ld      hl,txt_umcp
                call    um_inp
                jp      nz,ml_umenu
                call    um_build
                xor     a
                ld      b,a
                call    um_file
                jp      ml_umenu
; pk_path → um_ttl (파일 이름, 확장자 뺌, 12 글자까지)
um_fp_ttl:      ld      hl,pk_path
                ld      d,h
                ld      e,l
fp_l:           ld      a,(hl)
                or      a
                jr      z,fp_d
                inc     hl
                cp      '\'
                jr      nz,fp_l
                ld      d,h
                ld      e,l
                jr      fp_l
fp_d:           ex      de,hl
                ld      de,um_ttl
                ld      b,12
fp_c:           ld      a,(hl)
                or      a
                jr      z,fp_e
                cp      '.'
                jr      z,fp_e
                ld      (de),a
                inc     hl
                inc     de
                djnz    fp_c
fp_e:           xor     a
                ld      (de),a
                ret
; pk_path → um_pc = "경로 %s"
um_fp_cmd:      ld      hl,pk_path
                ld      de,um_pc
                call    copy_z
                ld      hl,txt_umps
                call    copy_z
                xor     a
                ld      (de),a
                ret
um_del:         ld      hl,txt_umdel
                call    qd_hint
                call    um_pick
                jp      c,um_show
                ld      b,a
                ld      a,1
                call    um_file
                jp      ml_umenu
um_ed:          ld      hl,txt_umed
                call    qd_hint
                call    um_pick
                jp      c,um_show
                ld      (um_ei),a
                call    um_ent                  ; HL = 항목
                push    hl
                inc     hl
                inc     hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ex      de,hl                   ; HL = 옛 명령
                ld      de,um_oc                ; (파일 고르기가 CPBUF 를 쓰므로 미리 따로 둠)
                call    strcpy_nul
                pop     hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 제목
                ld      hl,txt_umtp
                call    um_inp
                jp      nz,ml_umenu
                call    um_tget
                call    pick_file               ; 프로그램을 새로 고르거나(경로 %s), 직접 고침(옛 명령이 채워짐)
                ld      de,um_oc
                cp      2
                jr      z,ue_in
                or      a
                jr      nz,ue_in
                call    um_fp_cmd
                ld      de,um_pc
ue_in:          ld      hl,txt_umcp
                call    um_inp
                jp      nz,ml_umenu
                call    um_build
                ld      a,(um_ei)
                ld      b,a
                ld      a,2
                call    um_file
                jp      ml_umenu
; 글쇠 하나 → A = 번호 (없는 번호·해당 없음이면 C)
um_pick:        call    wait_key
                call    qd_idx
                ret     c
                ld      b,a
                ld      a,(um_n)
                ld      c,a
                ld      a,b
                cp      c
                jr      c,um_pk
                scf
                ret
um_pk:          or      a
                ret
; HL = 질문, DE = 미리 채울 글 (0 = 비움) → inbuf.  Z = 입력함, NZ = 취소·빈 입력
um_inp:         push    hl
                ld      a,d
                or      e
                jr      nz,ui_p
                xor     a
                ld      (inbuf),a
                jr      ui_g
ui_p:           ex      de,hl
                ld      de,inbuf
                call    strcpy_nul
ui_g:           ld      hl,inbuf
                call    strlen
                ld      (in_len),a
                or      a
                ld      a,0
                jr      z,ui_f
                inc     a
ui_f:           ld      (il_first),a
                ld      a,1
                ld      (il_raw),a
                pop     hl
                call    input_line
                push    af
                xor     a
                ld      (il_raw),a
                pop     af
                ret     nz
                call    trim_in
                ld      a,(in_len)
                or      a
                jr      z,ui_bad
                xor     a
                ret
ui_bad:         or      1
                ret
; inbuf → um_ttl (12 글자까지)
um_tget:        ld      hl,inbuf
                ld      de,um_ttl
                ld      b,12
ut_l:           ld      a,(hl)
                or      a
                jr      z,ut_e
                ld      (de),a
                inc     hl
                inc     de
                djnz    ut_l
ut_e:           xor     a
                ld      (de),a
                ret
; TAILBUF = "MENU 제목 = 명령" CR LF
um_build:       ld      de,TAILBUF
                ld      hl,txt_umm
                call    copy_z
                ld      hl,um_ttl
                call    copy_z
                ld      hl,txt_ume
                call    copy_z
                ld      hl,inbuf
                call    copy_z
                ld      a,13
                ld      (de),a
                inc     de
                ld      a,10
                ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                ret
; INI 고치기: A = 0 끝에 덧붙임 / 1 번호 B 의 MENU 줄 지움 / 2 번호 B 의 MENU 줄을 TAILBUF 로 바꿈
um_file:        ld      (uf_op),a
                ld      a,b
                ld      (uf_idx),a
                call    um_rdmenu               ; HL = 길이
uf_g:           ld      de,CPBUF
                add     hl,de
                ld      (hl),0
                ld      (uf_end),hl
                ld      a,(uf_op)
                or      a
                jp      z,uf_app
                xor     a
                ld      (uf_k),a
                ld      hl,CPBUF
uf_ln:          ld      a,(hl)
                or      a
                ret     z                       ; 못 찾음 (파일은 그대로)
                cp      21h
                jr      nc,uf_t
                inc     hl
                jr      uf_ln
uf_t:           ld      (uf_ls),hl
                push    hl
                ld      de,um_kw
                call    s7i_kw
                jr      nz,uf_no
                ld      a,(hl)
                cp      ' '
                jr      nz,uf_no
uf_eq:          ld      a,(hl)
                or      a
                jr      z,uf_no
                cp      10
                jr      z,uf_no
                cp      '='
                jr      z,uf_yes
                inc     hl
                jr      uf_eq
uf_no:          pop     hl
uf_sk:          ld      a,(hl)
                or      a
                ret     z
                inc     hl
                cp      10
                jr      nz,uf_sk
                jr      uf_ln
uf_yes:         pop     hl
                ld      a,(uf_k)
                ld      hl,uf_idx
                cp      (hl)
                jr      z,uf_hit
                ld      hl,uf_k
                inc     (hl)
                ld      hl,(uf_ls)
                jr      uf_sk
uf_hit:         ld      hl,(uf_ls)              ; 줄 끝 (LF 다음)
uf_le:          ld      a,(hl)
                or      a
                jr      z,uf_le2
                inc     hl
                cp      10
                jr      nz,uf_le
uf_le2:         ld      (uf_lv),hl
                ld      a,(uf_op)
                cp      1
                jr      nz,uf_rep
                ld      hl,(uf_end)             ; 지움: 꼬리(NUL 포함)를 줄 시작으로
                ld      de,(uf_lv)
                or      a
                sbc     hl,de
                inc     hl
                ld      b,h
                ld      c,l
                ld      hl,(uf_lv)
                ld      de,(uf_ls)
                ldir
                dec     de
                ld      (uf_end),de
                jp      uf_wr
uf_rep:         ld      hl,TAILBUF              ; 바꿈
                call    strlen
                ld      l,a
                ld      h,0
                ld      (uf_b),hl
                ld      hl,(uf_lv)
                ld      de,(uf_ls)
                or      a
                sbc     hl,de
                ld      (uf_a),hl
                ld      hl,(uf_end)
                ld      de,(uf_lv)
                or      a
                sbc     hl,de
                inc     hl
                ld      (uf_n),hl
                ld      hl,(uf_a)
                ld      de,(uf_b)
                or      a
                sbc     hl,de                   ; HL = 옛 길이 - 새 길이
                jr      z,uf_cp
                jr      c,uf_grow
                push    hl                      ; 줄어듦: 꼬리를 왼쪽으로
                ld      hl,(uf_ls)
                ld      de,(uf_b)
                add     hl,de
                ex      de,hl
                ld      hl,(uf_lv)
                ld      bc,(uf_n)
                ldir
                pop     de
                ld      hl,(uf_end)
                or      a
                sbc     hl,de
                ld      (uf_end),hl
                jr      uf_cp
uf_grow:        ld      hl,(uf_b)               ; 늘어남: 꼬리를 오른쪽으로 (뒤에서부터)
                ld      de,(uf_a)
                or      a
                sbc     hl,de
                ld      (uf_d),hl
                ld      de,(uf_end)
                add     hl,de
                ex      de,hl
                ld      hl,(uf_end)
                ld      bc,(uf_n)
                lddr
                ld      hl,(uf_end)
                ld      de,(uf_d)
                add     hl,de
                ld      (uf_end),hl
uf_cp:          ld      hl,TAILBUF
                ld      de,(uf_ls)
                ld      bc,(uf_b)
                ldir
                jr      uf_wr
uf_app:         ld      hl,(uf_end)             ; 덧붙임
                ld      de,CPBUF
                or      a
                sbc     hl,de
                jr      z,uf_a1
                ld      hl,(uf_end)
                dec     hl
                ld      a,(hl)
                cp      10
                jr      z,uf_a1
                ld      hl,(uf_end)
                ld      (hl),13
                inc     hl
                ld      (hl),10
                inc     hl
                ld      (uf_end),hl
uf_a1:          ld      hl,TAILBUF
                call    strlen
                ld      c,a
                ld      b,0
                ld      hl,TAILBUF
                ld      de,(uf_end)
                ldir
                ld      (uf_end),de
uf_wr:          ld      hl,k_menu               ; (0.12.14) INI 의 MENU 줄들을 CPBUF 글로 바꿔 씀
                ld      (iw_key),hl
                ld      hl,iw_mkey
                ld      de,gen_um
                jp      ini_wr
gen_um:         ld      de,CPBUF
                ld      hl,(uf_end)
                or      a
                sbc     hl,de
                ret     z
                jp      iw_blk
uf_op:          db      0
uf_idx:         db      0
uf_k:           db      0
uf_end:         dw      0
uf_ls:          dw      0
uf_lv:          dw      0
uf_a:           dw      0
uf_b:           dw      0
uf_n:           dw      0
uf_d:           dw      0
um_ei:          db      0
um_ttl:         ds      14
