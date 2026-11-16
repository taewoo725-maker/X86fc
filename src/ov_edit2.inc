; 만든이: KimTaeWoo
; X86FC 편집기 명령 (오버레이 10) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 편집기(오버레이 1)가 Ctrl+F 찾기 / F3·Ctrl+N 다음 / Ctrl+R 바꾸기 / Ctrl+G 줄 이동 을 받으면 이 칸으로 건너온다(ed_cmd).
;   편집기 상태(커서·끝·변수)는 쪽 3(edvars.inc)에 있고, 글은 매퍼 쪽(ed_seg)에 그대로 있다.  적재가 8000h 를 덮으므로 건너오기 전에
;   편집기가 쪽 2 를 원래대로 돌려 놓았고, 글을 만질 때만 여기서 다시 끼운다 (이 모듈은 3536 바이트 이하여야 함: 쪽 2 에 가려지지 않게).
; 끝나면 편집기(1번 칸, 진입 1)로 돌아감.  ed_rp = 1 이면 편집기가 "바꿀까요?" 를 묻고 ed_cmd 5(바꿈)·6(전부)·7(건너뜀) 로 다시 옴.
; 찾기는 영문 대소문자를 가리지 않음.  7H 의 한글 검색어는 UTF-8 로 바꿔 찾음.
; ----------------------------------------------------------------------
                include "edvars.inc"
                include "edundo.inc"
                include "ed2lib.inc"

ed2_cmd:        ld      a,(ed_cmd)
                cp      1
                jp      z,c_find
                cp      2
                jp      z,c_next
                cp      3
                jp      z,c_repl
                cp      4
                jp      z,c_goto
                cp      5
                jp      z,c_rthis
                cp      6
                jp      z,c_rall
                cp      7
                jp      z,c_fs
                cp      9
                jp      z,c_copy
                cp      10
                jp      z,c_cut
                cp      11
                jp      z,c_paste
                cp      12
                jp      z,c_dline
                cp      13
                jp      z,c_dup
                cp      14
                jp      z,c_dblk
                cp      15
                jp      z,c_undo
; ---- 글 입력: HL = 질문, DE = FS 또는 RS → NZ = ESC.  이전 글을 미리 채우고(첫 글자를 치면 지워짐), 돌아오면 UTF-8 로도 바꿔 둠
e2_ask:         ld      (ed_t4),hl
                ld      (ed_t3),de
                ld      a,(de)
                ld      (in_len),a
                ld      c,a
                ld      b,0
                inc     de
                ld      hl,inbuf
                ex      de,hl
                or      a
                jr      z,ea_z
                ldir
ea_z:           xor     a
                ld      (de),a
                inc     a
                ld      (il_first),a
                ld      hl,(ed_t4)
                call    input_line
                ret     nz
                ld      a,(in_len)
                cp      48
                jr      c,ea_s
                ld      a,47
ea_s:           ld      de,(ed_t3)
                ld      (de),a
                ld      c,a
                ld      b,0
                inc     de
                ld      hl,inbuf
                or      a
                jr      z,ea_u
                ldir
ea_u:           ld      hl,(ed_t3)              ; 입력 글(2바이트 한글 쌍) → UTF-8
                ld      a,(hl)
                ld      b,a
                ld      de,49
                add     hl,de
                push    hl                      ; 길이 칸
                inc     hl
                ex      de,hl                   ; DE = UTF-8 글
                ld      hl,inbuf
                ld      c,0
cv_l:           ld      a,b
                or      a
                jr      z,cv_d
                ld      a,(hl)
                inc     hl
                dec     b
                cp      80h
                jr      nc,cv_h
                call    cv_put
                jr      cv_l
cv_h:           ld      (ed_t4),a               ; 첫째 바이트
                ld      a,b
                or      a
                jr      z,cv_d                  ; 쌍이 잘렸으면 버림
                ld      a,(hl)                  ; 둘째 바이트
                inc     hl
                dec     b
                sub     80h
                ld      (ed_t4+1),a
                push    hl
                ld      a,(ed_t4)               ; 번호 = (첫째 - 80h) x 128 + (둘째 - 80h), 코드 = AC00h + 번호
                sub     80h
                ld      h,a
                ld      l,0
                srl     h
                rr      l
                ld      a,(ed_t4+1)
                or      l
                ld      l,a
                ld      a,h
                add     a,0ACh
                ld      h,a
                rrca
                rrca
                rrca
                rrca
                and     0Fh
                or      0E0h
                call    cv_put
                ld      a,h
                and     0Fh
                add     a,a
                add     a,a
                ld      h,a
                ld      a,l
                rlca
                rlca
                and     3
                or      h
                or      80h
                call    cv_put
                ld      a,l
                and     3Fh
                or      80h
                call    cv_put
                pop     hl
                jr      cv_l
cv_d:           xor     a
                ld      (de),a
                pop     hl
                ld      (hl),c
                xor     a
                ret
cv_put:         push    af
                ld      a,c
                cp      46
                jr      nc,cv_x
                pop     af
                ld      (de),a
                inc     de
                inc     c
                ret
cv_x:           pop     af
                ret

; ---- 찾음: 커서를 그 자리로, 화면 맨 위를 그 줄 처음으로
c_hit:          ld      (ed_cur),hl
                call    e2_settop
                call    e2_unmap
                jp      c_back
; ---- 명령들
c_find:         ld      hl,txt_find
                ld      de,FS
                call    e2_ask
                jp      nz,c_back
                ld      a,(fs_n)
                or      a
                jp      z,c_back
                jr      c_fs
c_next:         ld      a,(fs_n)
                or      a
                jr      z,c_find
c_fs:           ld      hl,(ed_cur)             ; 커서 한 칸 뒤부터 (바꾸기 중에는 끝까지만, 아니면 처음으로 한 번 감김)
                inc     hl
c_fs2:          push    hl
                call    e2_map
                pop     hl
                call    e2_scan
                jr      nc,c_hit
                ld      a,(big)
                or      a
                jr      z,c_f3
                ld      a,22                    ; 큰 파일: 뒤쪽 창을 차례로 읽으며 찾음 (오버레이 14)
                jp      to14
c_f3:           ld      a,(ed_rp)
                or      a
                jr      nz,c_nomore
                ld      hl,EDBUF
                call    e2_scan
                jp      nc,c_hit
c_nomore:       call    e2_unmap
                ld      hl,txt_nf
                ld      a,(ed_rp)
                or      a
                jr      z,c_nm1
                ld      hl,txt_done
c_nm1:          xor     a
                ld      (ed_rp),a
                call    e2_msg
                jp      c_back
c_repl:         ld      hl,txt_find
                ld      de,FS
                call    e2_ask
                jp      nz,c_back
                ld      a,(fs_n)
                or      a
                jp      z,c_back
                ld      hl,txt_with
                ld      de,RS
                call    e2_ask
                jp      nz,c_back
                ld      a,1
                ld      (ed_rp),a
                ld      hl,(ed_cur)             ; 커서 자리부터 (그 자리도 포함)
                jr      c_fs2
c_rthis:        call    e2_map                  ; 커서 자리의 찾은 글을 바꾸고 다음으로
                ld      hl,(ed_cur)
                call    e2_rep
                jr      c,c_full
                ld      hl,(ed_cur)
                ld      a,(rs_n)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      (ed_cur),hl
                ld      de,0
                ld      (ln_p),de
                jp      c_fs2
c_full:         xor     a
                ld      (ud_off),a
                call    e2_unmap
                xor     a
                ld      (ed_rp),a
                ld      hl,txt_full
                call    e2_msg
                jp      c_back
c_rall:         ld      a,(big)
                or      a
                ld      a,23                    ; 큰 파일: 전부 바꾸기는 오버레이 14
                jp      nz,to14
                call    e2_map
                ld      hl,LOG                  ; 전부 바꾸기는 되돌릴 수 없음: 기록을 비우고 쌓지 않음
                ld      (ud_top),hl
                ld      a,1
                ld      (ud_off),a
                ld      hl,0
                ld      (ed_cnt),hl
                ld      (ln_p),hl
                ld      hl,EDBUF
ra_l:           call    e2_scan
                jr      c,ra_d
                ld      (ed_cur),hl
                call    e2_rep
                jr      c,c_full
                ld      hl,(ed_cnt)
                inc     hl
                ld      (ed_cnt),hl
                ld      hl,(ed_cur)
                ld      a,(rs_n)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      (ed_cur),hl
                jr      ra_l
ra_d:           xor     a
                ld      (ud_off),a
                call    e2_unmap
                xor     a
                ld      (ed_rp),a
                ld      hl,EDBUF                ; 커서·화면 맨 위 = 글 처음
                ld      (ed_cur),hl
                ld      (ed_top),hl
                ld      de,errbuf
                ld      hl,(ed_cnt)
                call    put_dec16
                ld      hl,txt_repl
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                call    e2_msg
                jp      c_back

; ---- 줄 이동
c_goto:         xor     a
                ld      (in_len),a
                ld      (inbuf),a
                inc     a
                ld      (il_first),a
                ld      hl,txt_goto
                call    input_line
                jp      nz,c_back
                ld      hl,inbuf
                ld      de,0
gp_l:           ld      a,(hl)
                sub     '0'
                cp      10
                jr      nc,gp_d
                inc     hl
                push    hl
                ld      h,d
                ld      l,e
                add     hl,hl
                ld      b,h
                ld      c,l
                add     hl,hl
                add     hl,hl
                add     hl,bc
                ld      e,a
                ld      d,0
                add     hl,de
                ex      de,hl
                pop     hl
                jr      gp_l
gp_d:           ld      a,d
                or      e
                jr      nz,gp_n
                inc     e
gp_n:           ld      a,(big)
                or      a
                jr      z,gp_s
                ld      (bw_k),de               ; 큰 파일: 줄 이동은 오버레이 14
                ld      a,24
                jp      to14
gp_s:
                push    de
                call    e2_map
                pop     de
                ld      hl,EDBUF
                dec     de                      ; 건너뛸 줄바꿈 수 = 줄 번호 - 1
gp_f:           ld      a,d
                or      e
                jr      z,gp_h
                ld      bc,(ed_end)
                ld      a,c
                sub     l
                ld      c,a
                ld      a,b
                sbc     a,h
                ld      b,a
                or      c
                jr      z,gp_h
                ld      a,0Ah
                cpir
                jr      nz,gp_h
                dec     de
                jr      gp_f
gp_h:           ld      (ed_cur),hl
                ld      (ed_top),hl
                ld      hl,0
                ld      (ln_p),hl
                call    e2_unmap
                jp      c_back

; ---- (0.12.43) 선택 범위: NC = 있음 (HL = 처음, BC = 길이), C = 없음
e2_sel:         ld      hl,(sel_a)
                ld      a,h
                or      l
                jr      z,sl_no
                ld      de,(ed_cur)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,sl_1
                ex      de,hl
sl_1:           ex      de,hl                   ; HL = 큰 쪽, DE = 작은 쪽
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l
                ex      de,hl                   ; HL = 작은 쪽
                ld      a,b
                or      c
                jr      z,sl_no
                or      a
                ret
sl_no:          scf
                ret
; ---- 복사(Ctrl+C) / 자르기(Ctrl+X) / 선택 지우기(BS·DEL) / 붙이기(Ctrl+V)
c_copy:         call    e2_map
                call    e2_sel
                jr      c,cp_x
                call    e2_cl
                jr      c,cp_big
cp_ok:          ld      (clip_n),bc
                call    cl_put
                ld      hl,0
                ld      (sel_a),hl
cp_x:           call    e2_unmap
                jp      c_back
cp_big:         call    e2_unmap
                ld      hl,txt_big
                call    e2_msg
                jp      c_back
; ---- (0.12.50) 클립보드: HL = 글 안 원본, BC = 개수 → 클립보드 (별도 쪽이면 256바이트씩 CPBUF 를 거쳐).  글 쪽은 끼워진 채로 돌아옴
cl_put:         ld      a,(clip_seg)
                or      a
                jr      nz,cp_pg
                ld      de,CLIP
                ldir
                ret
cp_pg:          ld      (cl_a),hl
                ld      (cl_c),bc
                ld      hl,8000h
                ld      (cl_o),hl
cp_l:           ld      hl,(cl_c)
                ld      a,h
                or      l
                ret     z
                call    cl_n
                push    bc
                ld      hl,(cl_a)
                ld      de,CLIP
                ldir
                ld      (cl_a),hl
                ld      a,(clip_seg)
                call    mp_putp2
                pop     bc
                push    bc
                ld      hl,CLIP
                ld      de,(cl_o)
                ldir
                ld      (cl_o),de
                ld      a,(ed_seg)
                call    mp_putp2
                pop     bc
                ld      hl,(cl_c)
                or      a
                sbc     hl,bc
                ld      (cl_c),hl
                jr      cp_l
; 클립보드(clip_n 바이트) → 글 안 DE.  글 쪽은 끼워진 채로
cl_get:         ld      a,(clip_seg)
                or      a
                jr      nz,cg_pg
                ld      hl,CLIP
                ld      bc,(clip_n)
                ldir
                ret
cg_pg:          ld      (cl_a),de
                ld      hl,(clip_n)
                ld      (cl_c),hl
                ld      hl,8000h
                ld      (cl_o),hl
cg_l:           ld      hl,(cl_c)
                ld      a,h
                or      l
                ret     z
                call    cl_n
                push    bc
                ld      a,(clip_seg)
                call    mp_putp2
                pop     bc
                push    bc
                ld      hl,(cl_o)
                ld      de,CLIP
                ldir
                ld      (cl_o),hl
                ld      a,(ed_seg)
                call    mp_putp2
                pop     bc
                push    bc
                ld      hl,CLIP
                ld      de,(cl_a)
                ldir
                ld      (cl_a),de
                pop     bc
                ld      hl,(cl_c)
                or      a
                sbc     hl,bc
                ld      (cl_c),hl
                jr      cg_l
; HL = 남은 개수 → BC = min(HL, 256)
cl_n:           ld      a,h
                or      a
                jr      z,cn_s
                ld      bc,256
                ret
cn_s:           ld      b,h
                ld      c,l
                ret
; 길이 BC 가 클립보드에 들어가나: NC = 들어감 (HL·BC 보존)
e2_cl:          ld      a,(clip_seg)            ; (0.12.50) 별도 쪽이 있으면 크기 제한 없음
                or      a
                ret     nz
                push    hl
                ld      hl,CLIPMAX
                or      a
                sbc     hl,bc
                pop     hl
                ret
c_cut:          call    e2_map
                call    e2_sel
                jp      c,cp_x
                call    e2_cl
                jp      c,cp_big
                ld      (clip_n),bc
                ld      (sp_pos),hl
                ld      (sp_dl),bc
                call    cl_put
                jr      cd_go
c_dblk:         call    e2_map
                call    e2_sel
                jp      c,cp_x
                ld      (sp_pos),hl
                ld      (sp_dl),bc
cd_go:          ld      hl,0
                ld      (sp_sl),hl
                call    e2_splice
                ld      hl,(sp_pos)
                ld      (ed_cur),hl
                call    e2_fixtop
                call    e2_unmap
                jp      c_back
c_paste:        ld      hl,(clip_n)
                ld      a,h
                or      l
                jp      z,c_back
                call    e2_map
                call    e2_sel
                jr      c,pa_n
                ld      (sp_pos),hl             ; 선택이 있으면 먼저 지우고 그 자리에 붙임
                ld      (sp_dl),bc
                ld      hl,0
                ld      (sp_sl),hl
                call    e2_splice
                ld      hl,(sp_pos)
                ld      (ed_cur),hl
pa_n:           ld      hl,(ed_cur)
                ld      (sp_pos),hl
                ld      hl,0
                ld      (sp_dl),hl
                ld      hl,CLIP
                ld      a,(clip_seg)
                or      a
                jr      z,pa_s
                ld      hl,0                    ; 별도 쪽: 자리만 만들고 아래서 채움
pa_s:           ld      (sp_src),hl
                ld      hl,(clip_n)
                ld      (sp_sl),hl
                call    e2_splice
                jp      c,c_full
                ld      a,(clip_seg)
                or      a
                jr      z,pa_d
                ld      de,(sp_pos)
                call    cl_get
pa_d:
                ld      hl,(ed_cur)
                ld      de,(clip_n)
                add     hl,de
                ld      (ed_cur),hl
                call    e2_fixtop
                call    e2_unmap
                jp      c_back
; ---- 줄 지우기(Ctrl+Y) / 줄 복제(Ctrl+D): 줄 = 줄바꿈(LF)으로 끝나는 논리 줄
; 커서 줄의 처음(ed_t3)·끝+1(HL)
e2_line:        ld      hl,(ed_cur)
                ld      de,EDBUF
el_b:           or      a
                push    hl
                sbc     hl,de
                pop     hl
                jr      z,el_f
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,el_b
                inc     hl
el_f:           ld      (ed_t3),hl
                ld      de,(ed_end)
el_l:           push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                inc     hl
                cp      0Ah
                jr      nz,el_l
                ret
c_dline:        call    e2_map
                call    e2_line
                ld      de,(ed_t3)
                ld      (sp_pos),de
                or      a
                sbc     hl,de
                ld      (sp_dl),hl
                ld      a,h
                or      l
                jp      z,cp_x
                ld      hl,0
                ld      (sp_sl),hl
                jp      cd_go
c_dup:          call    e2_map
                call    e2_line                 ; HL = 줄 끝+1, ed_t3 = 줄 처음
                ld      de,(ed_t3)
                or      a
                sbc     hl,de                   ; HL = 줄 길이
                ld      a,h
                or      l
                jp      z,cp_x
                ld      (ed_cnt),hl
                add     hl,de                   ; HL = 줄 끝+1
                ld      (sp_pos),hl
                ld      bc,0                    ; BC = 덧붙인 줄바꿈 길이
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      z,du_2
                ld      hl,du_crlf              ; 마지막 줄이 줄바꿈 없이 끝나면 먼저 줄바꿈을 끼움
                ld      (sp_src),hl
                ld      hl,2
                ld      (sp_sl),hl
                ld      hl,0
                ld      (sp_dl),hl
                call    e2_splice
                jp      c,c_full
                ld      hl,(sp_pos)
                inc     hl
                inc     hl
                ld      (sp_pos),hl
                ld      bc,2
du_2:           push    bc
                ld      hl,(ed_t3)
                ld      (sp_src),hl
                ld      hl,(ed_cnt)
                ld      (sp_sl),hl
                ld      hl,0
                ld      (sp_dl),hl
                call    e2_splice
                pop     bc
                jp      c,c_full
                ld      hl,(ed_cnt)             ; 커서를 복제한 줄의 같은 자리로
                add     hl,bc
                ld      de,(ed_cur)
                add     hl,de
                ld      (ed_cur),hl
                call    e2_unmap
                jp      c_back
du_crlf:        db      0Dh,0Ah
; ---- (0.12.44) 되돌리기 (Ctrl+Z): 맨 위 기록을 꺼내 그 자리의 끼웠던 글을 지우고 지웠던 글을 다시 끼움
c_undo:         call    e2_map
                ld      hl,(ud_top)
                ld      a,h
                or      l
                jr      z,un_x
                ld      de,LOG
                or      a
                sbc     hl,de
                jr      z,un_x
                jr      c,un_x
                ld      hl,(ud_top)
                ld      de,-6
                add     hl,de                   ; HL = 꼬리
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                inc     hl
                ld      (sp_pos),de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)                  ; BC = 지웠던 길이
                inc     hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 끼웠던 길이
                ld      (sp_dl),de
                ld      (sp_sl),bc
                ld      hl,(ud_top)
                ld      de,6
                or      a
                sbc     hl,de
                or      a
                sbc     hl,bc                   ; HL = 저장해 둔 글 시작
                ld      (sp_src),hl
                ld      (ud_top),hl             ; 기록을 꺼냄
                ld      a,1
                ld      (ud_off),a
                call    e2_splice
                xor     a
                ld      (ud_off),a
                ld      hl,(sp_pos)             ; 커서 = 다시 끼운 글 끝
                ld      de,(sp_sl)
                add     hl,de
                ld      (ed_cur),hl
                call    e2_fixtop
un_x:           call    e2_unmap
                jp      c_back
; ---- 큰 파일 명령은 오버레이 14 (ov_edit5.inc): A = 명령
to14:           ld      (ed_cmd),a
                call    e2_unmap
                pop     hl
                ld      a,14
                ld      b,0
                jp      ov_go
txt_big:        db      "Block too big (760 bytes max)",0
txt_find:       db      "Find:",0
txt_with:       db      "Replace with:",0
txt_goto:       db      "Go to line:",0
txt_nf:         db      "Not found",0
txt_done:       db      "No more matches",0
txt_full:       db      "No room in the buffer",0
txt_repl:       db      " replaced",0
