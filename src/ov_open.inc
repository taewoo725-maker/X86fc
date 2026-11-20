do_setview:     call    cur_ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                ld      (en_p),hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      z,sv_f
                ld      hl,txt_notfile
                ld      (msg_p),hl
                ret
sv_f:           ld      hl,(en_p)
                ld      de,EN_EXT
                add     hl,de
                ld      de,sv_ext
                ld      bc,3
                ldir
                ld      a,(sv_ext)
                cp      ' '
                jr      nz,sv_hx
                ld      hl,txt_svnoext
                ld      (msg_p),hl
                ret
sv_hx:          ld      hl,(en_p)               ; 실행할 파일 경로 (%s 용)
                call    disp_name
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,RUNPATH
                call    mkpath
                call    vl_load
vl_manage:      xor     a
                ld      (vl_sel),a
vl_loop:        call    draw_all                ; (처음·목록이 바뀐 뒤에만 전체를 그림. ↑↓ 는 vl_key 로 두 줄만)
                call    vl_draw
vl_key:         call    wait_key
                cp      1Bh
                jr      z,vl_x
                cp      K_DOWN
                jr      z,vl_dn
                cp      K_UP
                jr      z,vl_up
                cp      0Dh
                jp      z,vl_run
                cp      K_INS
                jp      z,vl_add
                cp      K_DEL
                jp      z,vl_del
                cp      'd'
                jp      z,vl_def
                cp      'D'
                jp      z,vl_def
                jr      vl_key
vl_x:           jp      draw_all                ; 닫을 때 한 번 화면 전체를 다시 그려 팝업 자국을 지움
vl_dn:          ld a,(vl_sel)
                inc     a
                ld      hl,vl_n
                cp      (hl)
                jr      nc,vl_key
                call    vl_mv
                jr      vl_key
vl_up:          ld      a,(vl_sel)
                or      a
                jr      z,vl_key
                dec     a
                call    vl_mv
                jr      vl_key
; A = 새로 고른 번호: 예전 줄과 새 줄 두 줄만 다시 그림 (창·화면은 그대로)
vl_mv:          ld      b,a
                ld      a,(vl_sel)
                ld      c,a
                ld      a,b
                ld      (vl_sel),a
                push    bc
                ld      a,c
                call    vl_one
                pop     bc
                ld      a,b
vl_one:         add     a,2                     ; 항목 번호 → 팝업 줄 (0 위 테두리, 1 제목, 2~ 항목)
                ld      (vl_i),a
                ld      (vl_e),a
                jp      vd_l
vl_run:         ld      a,(vl_n)
                or      a
                jr      z,vl_key
                ld      a,(vl_sel)
                call    vl_slot                 ; DE = 고른 프로그램 명령 틀
                ex      de,hl
                ld      a,(hl)                  ; (0.12.27) 내부 명령 (#VIEW·#EDIT)은 본체의 rc_int 가 처리
                cp      '#'
                jp      z,rc_int
                xor     a
                ld      (cl_tmp),a
                ld      (hook_sh),a
                call    expand_cmd
                call    split_cmd
                call    rc_go                   ; 성공하면 돌아오지 않음
                jp      vl_loop
vl_add:         ld      a,(vl_n)
                cp      VLMAX
                jp      nc,vl_key
                call    pick_file               ; 프로그램을 찾아서 고름
                cp      2
                jp      z,vl_loop
                ld      de,inbuf
                or      a
                jr      nz,va_man
                ld      hl,pk_path              ; 고른 파일 + " %s" 로 미리 채움 (인자는 고쳐 쓸 수 있음)
                call    copy_z
                ld      hl,txt_vlarg
                call    copy_z
va_man:         xor     a
                ld      (de),a
                ld      hl,inbuf
                call    strlen
                ld      (in_len),a
                xor     a
                ld      (il_first),a
                ld      a,1
                ld      (il_raw),a
                call    draw_all
                ld      hl,txt_vlprog
                call    input_line
                push    af
                xor     a
                ld      (il_raw),a
                pop     af
                jp      nz,vl_loop
                call    trim_in
                ld      a,(in_len)
                or      a
                jp      z,vl_loop
                ld      a,(vl_n)
                call    vl_slot
                ld      hl,inbuf
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,vl_n
                inc     (hl)
                call    vl_save
                jp      vl_loop
vl_del:         ld      a,(vl_n)
                or      a
                jp      z,vl_loop
                ld      a,(vl_sel)              ; 고른 것 뒤의 항목들을 한 칸씩 앞으로
                ld      (vl_i),a
vd2_l:          ld      a,(vl_i)
                inc     a
                ld      hl,vl_n
                cp      (hl)
                jr      nc,vd2_e
                call    vl_slot
                push    de
                ld      a,(vl_i)
                call    vl_slot
                pop     hl
                ex      de,hl
                ld      bc,VLREC
                ldir
                ld      hl,vl_i
                inc     (hl)
                jr      vd2_l
vd2_e:          ld      hl,vl_n
                dec     (hl)
                ld      a,(vl_sel)
                ld      hl,vl_n
                cp      (hl)
                jr      c,vd2_s
                or      a
                jr      z,vd2_s
                dec     a
                ld      (vl_sel),a
vd2_s:          call    vl_save
                jp      vl_loop
vl_def:         ld      a,(vl_sel)              ; 고른 것을 맨 위(기본)로
                or      a
                jp      z,vl_loop
                call    vl_slot
                ld      hl,TAILBUF
                ex      de,hl
                ld      bc,VLREC
                ldir                            ; 임시로 보관
                ld      a,(vl_sel)
                ld      (vl_i),a
vdf_l:          ld      a,(vl_i)
                or      a
                jr      z,vdf_e
                call    vl_slot                 ; DE = [i]
                push    de
                ld      a,(vl_i)
                dec     a
                call    vl_slot                 ; DE = [i-1]
                pop     hl                      ; HL = [i]
                ex      de,hl
                ld      bc,VLREC
                ldir                            ; [i] <- [i-1]
                ld      hl,vl_i
                dec     (hl)
                jr      vdf_l
vdf_e:          xor     a
                call    vl_slot
                ld      hl,TAILBUF
                ld      bc,VLREC
                ldir
                xor     a
                ld      (vl_sel),a
                call    vl_save
                jp      vl_loop
; INI 를 다시 읽어 이 확장자의 VIEW 줄을 모두 빼고, 목록(VLBUF)을 새 VIEW 줄들로 붙여 저장
vl_save:        ld      hl,sv_match             ; (0.12.14) INI 에서 이 확장자 줄들을 지우고 새 목록을 붙임
                ld      de,gen_vs
                jp      ini_wr
gen_vs:         xor     a
                ld      (vl_i),a
gv_l:           ld      a,(vl_i)
                ld      hl,vl_n
                cp      (hl)
                ret     nc
                call    vl_slot
                push    de
                ld      de,CMDBUF
                ld      hl,sv_ext
                ld      b,3
gv_e:           ld      a,(hl)
                cp      ' '
                jr      z,gv_ee
                ld      (de),a
                inc     hl
                inc     de
                djnz    gv_e
gv_ee:          ld      hl,txt_sveq
                call    copy_z
                pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                call    iw_put
                ld      hl,vl_i
                inc     (hl)
                jr      gv_l
; 목록 팝업 그리기: 높이 = (줄 수 또는 1) + 5, 너비 70, 가운데
vl_draw:        ld      a,(vl_n)
                or      a
                jr      nz,vd_a
                ld      a,1
vd_a:           ld      (vl_c),a
                add     a,5
                ld      (vl_h),a
                ld      b,a
                ld      a,24
                sub     b
                srl     a
                ld      (vl_r0),a
                ld      de,sv_prm               ; 제목: "Open .EXT files with"
                ld      hl,txt_vlt1
                call    copy_z
                ld      hl,sv_ext
                ld      b,3
vd_t:           ld      a,(hl)
                cp      ' '
                jr      z,vd_te
                ld      (de),a
                inc     hl
                inc     de
                djnz    vd_t
vd_te:          ld      hl,txt_vlt2
                call    copy_z
                xor     a
                ld      (de),a
                ld      (vl_i),a
                ld      a,(vl_h)
                dec     a
                ld      (vl_e),a                ; 그릴 마지막 줄 (전체 = 맨 아래 테두리)
vd_l:           ld      hl,qd_tmp               ; 한 줄(70 칸) 바탕
                ld      a,(vl_i)
                or      a
                jr      nz,vd_n0
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      vd_f
vd_n0:          ld      b,a
                ld      a,(vl_h)
                dec     a
                cp      b
                jr      nz,vd_n5
                ld      c,CH_BL
                ld      d,CH_HBAR
                ld      e,CH_BR
                jr      vd_f
vd_n5:          ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
vd_f:           ld      (hl),c
                inc     hl
                ld      b,68
vd_fl:          ld      (hl),d
                inc     hl
                djnz    vd_fl
                ld      (hl),e
                ld      a,(vl_i)                ; 글자 넣기
                cp      1
                jr      nz,vd_x2
                ld      hl,sv_prm
                ld      de,qd_tmp+3
                call    copy_z
                jr      vd_wr
vd_x2:          ld      b,a
                ld      a,(vl_h)
                sub     2
                cp      b
                jr      nz,vd_x3
                ld      hl,txt_vlhint
                ld      de,qd_tmp+3
                call    copy_z
                jr      vd_wr
vd_x3:          ld      a,b                     ; 2 .. c+1 : 항목
                cp      2
                jr      c,vd_wr
                sub     2
                ld      b,a
                ld      a,(vl_c)
                cp      b
                jr      z,vd_wr
                jr      c,vd_wr
                ld      a,(vl_n)
                or      a
                jr      nz,vd_e1
                ld      hl,txt_vlnone
                ld      de,qd_tmp+3
                call    copy_z
                jr      vd_wr
vd_e1:          ld      a,b
                ld      hl,vl_sel
                cp      (hl)
                ld      a,' '
                jr      nz,vd_e2
                ld      a,'>'
vd_e2:          ld      (qd_tmp+3),a
                ld      a,b
                call    vl_slot
                ex      de,hl
                call    vd_nm                   ; (0.12.27) 내부 명령은 이름으로 보임
                ld      de,qd_tmp+5
                ld      b,62
vd_ec:          ld      a,(hl)
                or      a
                jr      z,vd_wr
                ld      (de),a
                inc     hl
                inc     de
                djnz    vd_ec
vd_wr:          ld      a,(vl_r0)
                ld      b,a
                ld      a,(vl_i)
                add     a,b
                call    row_addr
                ld      de,5
                add     hl,de
                ld      de,qd_tmp
                ld      b,70
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ld      a,(vl_i)
                inc     a
                ld      (vl_i),a
                ld      b,a
                ld      a,(vl_e)
                cp      b
                jp      nc,vd_l
                ret
txt_vlt1:       db      "Open .",0
txt_vlt2:       db      " files with",0
txt_vlhint:     db      "Enter run   INS add   DEL remove   D make default   Esc close",0
txt_vlnone:     db      "(no program registered - press INS to add one)",0
txt_vlprog:     db      "Program (%s = file)",0
txt_vlarg:      db      " %s",0
vl_sel:         db      0
vl_i:           db      0
vl_c:           db      0
vl_h:           db      0
vl_r0:          db      0
vl_e:           db      0
txt_svnoext:    db      "This file has no extension",0
txt_svp1:       db      "Viewer for .",0
txt_svp2:       db      "  (%s = file, empty = remove)",0
txt_svview:     db      "VIEW ",0
txt_sveq:       db      " = ",0
txt_svdone:     db      "Viewer saved in X86FC.INI",0
sv_prm:         ds      48
sv_len:         dw      0

; ---- (0.12.9) F1 도움말: 본체에서 옮겨 옴 (진입 2)
show_help:      call    h_off
                call    clear_screen
                ld      hl,help_txt
                xor     a
                ld      (hp_r),a
hp_l:           ld      a,(hl)
                cp      0FFh
                jr      z,hp_wait
                push    hl
                ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                pop     hl
                ld      de,hbuf+2
                call    copy_z
                inc     hl                      ; NUL 건너뜀
                push    hl
                ld      a,(hp_r)
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      a,(hp_r)
                inc     a
                ld      (hp_r),a
                pop     hl
                jr      hp_l
hp_wait:        call    wait_key
                jp      h_on
hp_r:           db      0
; F1 안내 (한 줄 = 글 하나, 끝 0FFh)
help_txt:       db      "X86FC 0.12.56 - Help",0
                db      " ",0
                db      "FUNCTION KEYS   (F6-F10 = Shift+F1 ... Shift+F5)",0
                db      "F1          Help                      F6          Rename",0
                db      "F2          View (text only)          F7          Make folder",0
                db      "F3          View + Edit               F8          Sort: name/ext/size/date",0
                db      "F4          Screen: text / 7 / 7H     F9          Drive (active panel)",0
                db      "F5          MSX-BASIC                 F10         Exit",0
                db      "OTHER KEYS",0
                db      "Up Down     Move cursor               Tab         Other panel",0
                db      "Left Right  Page up / down            Enter       Open / run / link program",0
                db      "Home        Root folder               Ctrl+Enter  Name to command line",0
                db      "A-Z 0-9     Jump to name              BS          Parent folder",0
                db      "Space       Select / unselect         Ctrl+A      Select all",0
                db      "INS         Copy                      DEL         Delete",0
                db      "SELECT      Move                      /           Command line",0
                db      "Ctrl+B      Quick-Dir                 Ctrl+F      Find file",0
                db      "Ctrl+W      Folder tree               Ctrl+G      User menu",0
                db      "Ctrl+F3     Programs of this type                 ",0
                db      "ESC         Show the DOS screen (any key to come back)",0
                db      "Han/Eng key  Han/Eng while typing a name (7H, Hangul 2-set)",0
                db      "Editor (F3) ^F Find  F3 Next  ^R Replace  ^G Goto line  Shift+Arrows or ^B Select",0
                db      "            ^C Copy  ^X Cut  ^V Paste  ^Y Delete line  ^D Duplicate line  ^Z Undo  ^S Save",0
                db      "Press any key",0
                db      0FFh

; HL = 항목 명령 → HL = 목록에 보일 글 (#VIEW·#EDIT 는 이름, 그 밖에는 그대로)
vd_nm:          ld      a,(hl)
                cp      '#'
                ret     nz
                push    hl
                ld      de,txt_i_view
                call    int_cmp
                ld      de,txt_vwi
                jr      z,vn_s
                pop     hl
                push    hl
                ld      de,txt_i_edit
                call    int_cmp
                ld      de,txt_edi
                jr      z,vn_s
                pop     hl
                ret
vn_s:           pop     hl
                ex      de,hl
                ret
txt_vwi:        db      "View (internal)",0
txt_edi:        db      "Edit (internal)",0
