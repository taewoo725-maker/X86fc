; 만든이: KimTaeWoo
; X86FC 편집기 명령 공통 루틴 (오버레이 10·14 가 같이 씀) - SPDX-License-Identifier: GPL-3.0-or-later
; 쪽 2 끼우기/되돌리기, 편집기로 돌아가기(c_back), 찾기 비교(e2_scan), 줄 처음 잡기, 끼워 넣기·지우기(e2_splice)
c_back:         pop     hl                      ; ov_ret 버리고 편집기로
                xor     a
                ld      b,a
                inc     a
                jp      ov_go

; ---- 쪽 2 끼우기/되돌리기, 알림
mp_putp2:       ld      hl,(mp_put)
                jp      (hl)
mp_getp2:       ld      hl,(mp_get)
                jp      (hl)
e2_map:         ld      a,(ed_seg)
                jr      e2_put
e2_unmap:       ld      a,(ed_old)
e2_put:         ld      hl,(mp_put)
                jp      (hl)
e2_msg:         ld      de,txt_ppok
                jp      popup_ask

; ---- 찾기: HL = 시작 자리 → NC = 찾음 (HL = 그 자리), C = 없음.  영문 대소문자 무시.  글은 쪽 2 에 끼워진 상태
e2_scan:        ld      a,(fs_n)
                or      a
                jr      z,sc_no
                ld      c,a
                ld      b,0
                push    hl
                ld      hl,(ed_end)
                or      a
                sbc     hl,bc
                jr      c,sc_np                 ; 글이 찾는 글보다 짧음
                inc     hl
                ld      (ed_t1),hl              ; 이 자리 아래까지만 (끝에서 길이만큼 앞)
                pop     hl
sc_l:           ld      de,(ed_t1)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,sc_no
                push    hl
                ld      de,fs_str
                ld      a,(fs_n)
                ld      b,a
sc_c:           ld      a,(de)
                call    lc_a
                ld      c,a
                ld      a,(hl)
                call    lc_a
                cp      c
                jr      nz,sc_x
                inc     hl
                inc     de
                djnz    sc_c
                pop     hl
                or      a
                ret
sc_x:           pop     hl
                inc     hl
                jr      sc_l
sc_np:          pop     hl
sc_no:          scf
                ret
lc_a:           cp      'A'
                ret     c
                cp      'Z'+1
                ret     nc
                or      20h
                ret

; 화면 맨 위 = 커서가 있는 (논리) 줄 처음
e2_settop:      ld      hl,(ed_cur)
                ld      de,EDBUF
c_hl:           or      a
                push    hl
                sbc     hl,de
                pop     hl
                jr      z,c_ht
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,c_hl
                inc     hl
c_ht:           ld      (ed_top),hl
                ret
; 맨 위가 지운·고친 자리(sp_pos) 이후면 다시 잡음
e2_fixtop:      ld      hl,(ed_top)
                ld      de,(sp_pos)
                or      a
                sbc     hl,de
                ret     c
                jr      e2_settop

; ---- HL = 찾은 자리: 거기의 찾는 글(fs_n 바이트)을 바꿀 글(rs_n 바이트)로 → C = 자리 모자람
e2_rep:         ld      (sp_pos),hl
                ld      a,(fs_n)
                ld      l,a
                ld      h,0
                ld      (sp_dl),hl
                ld      a,(rs_n)
                ld      l,a
                ld      (sp_sl),hl
                ld      hl,rs_str
                ld      (sp_src),hl
; ---- 끼워 넣기·지우기: sp_pos 자리의 sp_dl 바이트를 sp_src 의 sp_sl 바이트로 바꿈 → C = 자리 모자람.  글은 쪽 2 에 끼워진 상태
e2_splice:      ld      hl,(ed_end)
                ld      de,(sp_dl)
                or      a
                sbc     hl,de
                ld      bc,(sp_sl)
                add     hl,bc
                ld      (ed_t4),hl              ; 새 끝
                ld      de,EDBUF+EDMAX
                or      a
                sbc     hl,de
                jr      c,sp_go
                jr      z,sp_go
                scf
                ret
sp_go:          call    ud_push                 ; (0.12.44) 되돌리기 기록
                ld      hl,(sp_pos)             ; 뒤쪽(지운 자리 다음 ~ 끝)을 새 자리로 밈
                ld      de,(sp_dl)
                add     hl,de
                ex      de,hl                   ; DE = 원래 뒤쪽 시작
                ld      hl,(ed_end)
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l                     ; BC = 뒤쪽 길이
                ex      de,hl                   ; HL = 원래 뒤쪽 시작
                ld      de,(sp_pos)
                push    hl
                ld      hl,(sp_sl)
                add     hl,de
                ex      de,hl                   ; DE = 새 뒤쪽 시작
                pop     hl
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,sp_cp
                jr      nc,sp_fw                ; 원래 > 새: 앞으로 당김
                ld      a,b                     ; 원래 < 새: 뒤에서부터 밈
                or      c
                jr      z,sp_cp
                add     hl,bc
                dec     hl
                ex      de,hl
                add     hl,bc
                dec     hl
                ex      de,hl
                lddr
                jr      sp_cp
sp_fw:          ld      a,b
                or      c
                jr      z,sp_cp
                ldir
sp_cp:          ld      bc,(sp_sl)
                ld      a,b
                or      c
                jr      z,sp_e
                ld      hl,(sp_src)
                ld      a,h                     ; (0.12.50) 0 = 자리만 만들고 글은 호출한 쪽이 채움
                or      l
                jr      z,sp_e
                ld      de,(sp_pos)
                ldir
sp_e:           ld      hl,(ed_t4)
                ld      (ed_end),hl
                ld      a,1
                ld      (ed_mod),a
                ld      hl,0
                ld      (ln_p),hl               ; 줄 번호 표시 처음부터 다시
                ld      (sel_a),hl              ; 선택 해제
                or      a
                ret

