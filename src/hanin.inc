; 만든이: KimTaeWoo
; X86FC 한글 입력 (두벌식 조합 오토마타) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 7H 에서 이름 바꾸기·폴더 만들기(input_line)에 한글을 넣기 위한 코드.  한/영 전환 = MSX 본래 한/영(KANA/CODE) 키.
; 입력 글(inbuf) 안의 한글 한 글자 = 두 바이트 (80h + (번호 >> 7), 80h + (번호 AND 7Fh)),
;   번호 = (초성 x 21 + 중성) x 28 + 종성 = 완성형 11172자 순번 (목록·글꼴 파일과 같은 표현).
; 조합 중인 글자는 inbuf 맨 끝 두 바이트에 있고, 상태(hk_st)와 초성·중성·종성 번호를 따로 가짐:
;   0 = 조합 중 아님, 1 = 초성만 (모음 대신 ㅡ 모양으로 보임), 2 = 초성+중성, 3 = 초성+중성+종성.
; 홀 모음(앞에 자음이 없을 때)은 ㅇ + 모음 글자가 됨.
; 키: A = 키.  결과 C 깃발 = 처리함 (il_draw 로 다시 그림), NC = 처리 안 함 (A 그대로, 조합은 끝냄)
; ----------------------------------------------------------------------
; 한/영 = MSX 본래 한/영(KANA/CODE) 키 = 키보드 행렬 6행 비트 4.  BIOS 가 이 키로 KANAST(FCACh, 0 = 꺼짐 / 0 아님 = 켜짐)를 바꾸고
;   KANA 램프(PSG 15번 레지스터 비트 7, 0 = 켜짐)를 켜고 끔.  입력 창은 KANAST 를 읽어 한글 상태를 정함 (il_wait).
;   BIOS 는 한글 상태에서 글쇠를 한글·가나 코드로 바꿔 주므로, 알파벳은 키보드 행렬(SNSMAT 2~5행)에서 직접 읽음.
KANAST          equ     0FCACh
hk_key:         call    hk_sync                 ; 한/영 상태 맞춤 (A 보존)
hk_nt:          ld      c,a
                ld      a,(hk_on)
                or      a
                jp      z,hk_no
                ld      a,c
                cp      08h
                jp      z,hk_bs
                and     0DFh
                sub     'A'
                cp      26
                jp      nc,hk_no
                ld      b,a                     ; B = 글자 번호 0~25
                ld      a,c
                cp      'a'
                ld      hl,hk_map               ; 소문자 표
                jr      nc,hk_l1
                ld      hl,hk_mapS              ; 대문자(Shift) 표
hk_l1:          ld      e,b
                ld      d,0
                add     hl,de
                ld      b,(hl)                  ; B = 값 (비트 7 = 모음, 아래 = 번호)
                ld      a,(il_first)            ; 첫 글자면 기존 글을 지움
                or      a
                jr      z,hk_nf
                xor     a
                ld      (il_first),a
                ld      (in_len),a
                ld      (inbuf),a
                ld      (hk_st),a
hk_nf:          ld      a,b
                bit     7,a
                jr      nz,hk_vow
; ---- 자음 (A = 초성 번호)
                ld      (hk_c),a
                ld      a,(hk_st)
                cp      2
                jr      z,hz_hc2
                cp      3
                jr      nz,hk_new               ; 0, 1: 새 글자
                ld      a,(hk_jong)             ; 3: 겹받침이 되나?
                ld      b,a
                ld      a,(hk_c)
                ld      c,a
                ld      hl,hk_cj
                ld      e,11
                call    hk_fab
                jr      nz,hk_new
                inc     hl
                inc     hl
                ld      a,(hl)
                ld      (hk_jong),a
                jp      hk_put
hz_hc2:            ld      a,(hk_c)                ; 2: 받침이 되나?
                ld      e,a
                ld      d,0
                ld      hl,hk_c2j
                add     hl,de
                ld      a,(hl)
                cp      0FFh
                jr      z,hk_new
                ld      (hk_jong),a
                ld      a,3
                ld      (hk_st),a
                jp      hk_put
hk_new:         ld      a,(hk_c)                ; 새 글자 (초성)
                ld      (hk_cho),a
                ld      a,0FFh
                ld      (hk_jung),a
                xor     a
                ld      (hk_jong),a
                inc     a
                ld      (hk_st),a
                jp      hk_app
; ---- 모음 (A = 80h + 중성 번호)
hk_vow:         and     7Fh
                ld      (hk_c),a
                ld      a,(hk_st)
                or      a
                jr      z,hz_hv_ie
                cp      1
                jr      z,hz_hv1
                cp      2
                jr      z,hz_hv2
                ld      a,(hk_jong)             ; 3: 받침을 다음 글자 초성으로 넘김
                ld      b,a
                ld      hl,hk_cj
                ld      e,11
                call    hk_fr
                jr      nz,hz_hv3s
                ld      a,(hl)                  ; 겹받침: 왼쪽은 받침으로 남고 오른쪽이 초성
                ld      (hk_jong),a
                inc     hl
                ld      c,(hl)
                jr      hz_hv3g
hz_hv3s:           ld      a,(hk_jong)
                ld      e,a
                ld      d,0
                ld      hl,hk_j2c
                add     hl,de
                ld      c,(hl)
                xor     a
                ld      (hk_jong),a
hz_hv3g:           push    bc
                call    hk_put
                pop     bc
                ld      a,c
                ld      (hk_cho),a
                jr      hz_hv_n2
hz_hv2:            ld      a,(hk_jung)             ; 2: 겹모음이 되나?
                ld      b,a
                ld      a,(hk_c)
                ld      c,a
                ld      hl,hk_cv
                ld      e,7
                call    hk_fab
                jr      nz,hz_hv_ie
                inc     hl
                inc     hl
                ld      a,(hl)
                ld      (hk_jung),a
                jp      hk_put
hz_hv1:            ld      a,(hk_c)                ; 1: 초성 + 모음
                ld      (hk_jung),a
                ld      a,2
                ld      (hk_st),a
                jp      hk_put
hz_hv_ie:          ld      a,11                    ; 홀모음 / 모음 두 번: ㅇ + 모음
                ld      (hk_cho),a
hz_hv_n2:          ld      a,(hk_c)
                ld      (hk_jung),a
                xor     a
                ld      (hk_jong),a
                ld      a,2
                ld      (hk_st),a
                jp      hk_app
hk_no:          xor     a
                ld      (hk_st),a
                ld      a,c
                or      a                       ; NC, A = 키
                ret
; ---- 조합 중 BS: 종성 -> 중성 -> 초성 -> 글자 순으로 한 단계씩 지움
hk_bs:          ld      a,(hk_st)
                or      a
                jp      z,hk_no
                cp      3
                jr      z,hz_hb3
                cp      2
                jr      z,hz_hb2
                xor     a                       ; 1: 글자 지움
                ld      (hk_st),a
                ld      a,(in_len)
                sub     2
                ld      (in_len),a
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      (hl),0
                scf
                ret
hz_hb3:            ld      a,(hk_jong)
                ld      b,a
                ld      hl,hk_cj
                ld      e,11
                call    hk_fr
                jr      nz,hz_hb3n
                ld      a,(hl)                  ; 겹받침 -> 왼쪽만 남김 (그대로 3)
                ld      (hk_jong),a
                jp      hk_put
hz_hb3n:           xor     a
                ld      (hk_jong),a
                ld      a,2
                ld      (hk_st),a
                jp      hk_put
hz_hb2:            ld      a,(hk_jung)
                ld      b,a
                ld      hl,hk_cv
                ld      e,7
                call    hk_fr
                jr      nz,hz_hb2n
                ld      a,(hl)                  ; 겹모음 -> 앞 모음만
                ld      (hk_jung),a
                jp      hk_put
hz_hb2n:           ld      a,0FFh
                ld      (hk_jung),a
                ld      a,1
                ld      (hk_st),a
                jp      hk_put
; ---- 표 찾기.  HL = 표(3바이트 항목), E = 항목 수.  찾으면 Z, HL = 항목 처음
; hk_fab: 앞 두 바이트 = B, C 인 항목     hk_fr: 셋째 바이트 = B 인 항목
hk_fab:         ld      a,(hl)
                cp      b
                jr      nz,hz_fab_n
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      c
                ret     z
hz_fab_n:          inc     hl
                inc     hl
                inc     hl
                dec     e
                jr      nz,hk_fab
                or      1
                ret
hk_fr:          inc     hl
                inc     hl
                ld      a,(hl)
                dec     hl
                dec     hl
                cp      b
                ret     z
                inc     hl
                inc     hl
                inc     hl
                dec     e
                jr      nz,hk_fr
                or      1
                ret
; ---- 상태 -> 글자 두 바이트 (B = 첫째, C = 둘째)
hk_calc:        ld      a,(hk_jung)
                cp      0FFh
                jr      nz,hz_kc_1
                ld      a,18                    ; 초성만: ㅡ
hz_kc_1:           ld      e,a
                ld      d,0
                ld      a,(hk_cho)
                ld      c,a
                ld      b,0
                ld      h,b
                ld      l,c
                add     hl,hl
                add     hl,hl
                add     hl,bc                   ; x 5
                add     hl,hl
                add     hl,hl
                add     hl,bc                   ; x 21
                add     hl,de                   ; + 중성
                add     hl,hl
                add     hl,hl                   ; x 4
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; x 32
                or      a
                sbc     hl,de                   ; x 28
                ld      a,(hk_jong)
                ld      e,a
                ld      d,0
                add     hl,de                   ; 번호
                ld      a,l
                and     7Fh
                or      80h
                ld      c,a
                ld      a,l
                rlca
                ld      a,h
                rla
                or      80h
                ld      b,a
                ret
; 조합 중인 글자(inbuf 맨 끝 두 바이트)를 지금 상태로 바꿈
hk_put:         call    hk_calc
                ld      a,(in_len)
                sub     2
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      (hl),b
                inc     hl
                ld      (hl),c
                scf
                ret
; 새 글자를 inbuf 끝에 붙임 (가득 차면 조합 끝)
hk_app:         ld      a,(in_len)
                cp      IN_MAX-1
                jr      c,hz_ha_ok
                xor     a
                ld      (hk_st),a
                scf
                ret
hz_ha_ok:          call    hk_calc
                ld      a,(in_len)
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      (hl),b
                inc     hl
                ld      (hl),c
                inc     hl
                ld      (hl),0
                add     a,2
                ld      (in_len),a
                scf
                ret
; 마지막 글자의 바이트 수 (1 또는 2) -> A  (in_len > 0)
hk_clen:        ld      hl,inbuf
                ld      a,(in_len)
                ld      b,a
hz_kl_l:           ld      c,1
                ld      a,(hl)
                cp      80h
                jr      c,hz_kl_s
                ld      a,b
                cp      2
                jr      c,hz_kl_s
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      80h
                jr      c,hz_kl_s
                ld      c,2
hz_kl_s:           ld      a,b
                sub     c
                jr      z,hz_kl_e
                ld      b,a
                ld      e,c
                ld      d,0
                add     hl,de
                jr      hz_kl_l
hz_kl_e:           ld      a,c
                ret
; 입력 창에 보일 시작 위치 (끝 43 바이트, 글자를 쪼개지 않게) -> HL
hk_wst:         ld      a,(in_len)
                sub     43
                jr      nc,hz_ws_a
                xor     a
hz_ws_a:           ld      c,a
                ld      hl,inbuf
                ld      b,0
hz_ws_l:           ld      a,b
                cp      c
                ret     nc
                ld      a,(hl)
                cp      80h
                jr      c,hz_ws_s
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      80h
                jr      c,hz_ws_s
                inc     hl
                inc     b
hz_ws_s:           inc     hl
                inc     b
                jr      hz_ws_l
; 한/영 상태 hk_xon (0/1) 은 입력 창이 가진다.  한/영 키를 누르면 BIOS 가 KANAST 를 켜므로 (il_wait 가 봄) 그때 hk_xon 을 뒤집고
;   KANAST 를 0 으로 되돌려 BIOS 가 글쇠를 계속 ASCII 로 주게 하고 (BIOS 의 글쇠 버퍼가 빠른 타자를 받아 줌),
;   KANA 램프(PSG 15번 레지스터 비트 7, 0 = 켜짐)는 hk_xon 에 맞춤.
; hk_on = hk_xon 이고 7H 이고 명령줄이 아닐 때 1.  바뀌면 조합 중이던 글자를 끝냄.  A 보존
hk_sync:        push    af
                ld      a,(hk_xon)
                or      a
                ld      b,0
                jr      z,hy_a
                inc     b
hy_a:           ld      a,(s7_hm)
                or      a
                jr      nz,hy_b
                ld      b,0
hy_b:           ld      a,(il_raw)
                or      a
                jr      z,hy_c
                ld      b,0
hy_c:           ld      a,(hk_on)
                cp      b
                jr      z,hy_x
                ld      a,b
                ld      (hk_on),a
                xor     a
                ld      (hk_st),a
hy_x:           pop     af
                ret
; 입력 창 글쇠 기다림 -> A = 글쇠.  A = 0 이면 한/영 키가 눌려 상태가 바뀐 것 (다시 그림)
il_wait:
iw_lp:          ld      a,(KANAST)
                or      a
                jr      z,iw_nk
                xor     a
                ld      (KANAST),a
                ld      a,(hk_xon)
                xor     1
                ld      (hk_xon),a
                call    hk_led
                xor     a
                ret
iw_nk:          ld      ix,CHSNS
                call    bios
                jr      z,iw_lp
                ld      ix,CHGET
                jp      bios
; KANA 램프 = hk_xon (PSG 15번 레지스터 비트 7: 0 = 켜짐, 나머지 비트는 그대로)
hk_led:         ld      a,15
                ld      ix,RDPSG
                call    bios
                and     7Fh
                ld      b,a
                ld      a,(hk_xon)
                or      a
                ld      a,b
                jr      nz,hl_s
                or      80h
hl_s:           ld      e,a
                ld      a,15
                ld      ix,WRTPSG
                call    bios
                ret
; 입력 창을 떠날 때: 한글 상태·KANAST 를 끄고 램프도 끔 (잠금이 남으면 파일 관리자의 글자 이동이 한글·가나 코드로 바뀜)
hk_off:         ld      a,(KANAST)
                ld      b,a
                ld      a,(hk_xon)
                or      b
                ret     z
                xor     a
                ld      (hk_xon),a
                ld      (hk_on),a
                ld      (KANAST),a
                jp      hk_led
hk_xon:         db      0
; 입력 창 안내 글 -> HL
hk_hint:        call    hk_sync
                ld      hl,txt_inh
                ld      a,(s7_hm)
                or      a
                ret     z
                ld      a,(il_raw)
                or      a
                ret     nz
                ld      hl,txt_inhe
                ld      a,(hk_on)
                or      a
                ret     z
                ld      hl,txt_inhk
                ret
txt_inhe:       db      "[Enter] OK  [Esc] Cancel  [Han/Eng key] ENG",0
txt_inhk:       db      "[Enter] OK  [Esc] Cancel  [Han/Eng key] HAN",0
hk_on:          db      0
hk_st:          db      0
hk_cho:         db      0
hk_jung:        db      0
hk_jong:        db      0
hk_c:           db      0
; 키 표 (비트 7 = 모음, 아래 = 번호): 초성 ㄱ0 ㄲ1 ㄴ2 ㄷ3 ㄸ4 ㄹ5 ㅁ6 ㅂ7 ㅃ8 ㅅ9 ㅆ10 ㅇ11 ㅈ12 ㅉ13 ㅊ14 ㅋ15 ㅌ16 ㅍ17 ㅎ18
;   중성 ㅏ0 ㅐ1 ㅑ2 ㅒ3 ㅓ4 ㅔ5 ㅕ6 ㅖ7 ㅗ8 ㅘ9 ㅙ10 ㅚ11 ㅛ12 ㅜ13 ㅝ14 ㅞ15 ㅟ16 ㅠ17 ㅡ18 ㅢ19 ㅣ20
;               a    b    c   d   e  f   g   h    i    j    k    l    m
hk_map:         db      6,   91h, 14, 11, 3, 5,  18, 88h, 82h, 84h, 80h, 94h, 92h
;               n    o    p    q  r  s  t  u    v   w   x   y    z
                db      8Dh, 81h, 85h, 7, 0, 2, 9, 86h, 17, 12, 16, 8Ch, 15
hk_mapS:        db      6,   91h, 14, 11, 4, 5,  18, 88h, 82h, 84h, 80h, 94h, 92h
                db      8Dh, 83h, 87h, 8, 1, 2, 10, 86h, 17, 13, 16, 8Ch, 15
; 초성 -> 받침 번호 (FFh = 받침 못 됨)
hk_c2j:         db      1,2,4,7,0FFh,8,16,17,0FFh,19,20,21,22,0FFh,23,24,25,26,27
; 받침 -> 초성 번호 (홑받침만)
hk_j2c:         db      0FFh,0,1,0FFh,2,0FFh,0FFh,3,5,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,6,7,0FFh,9,10,11,12,14,15,16,17,18
; 겹받침 (왼쪽 받침, 오른쪽 초성, 결과 받침)
hk_cj:          db      1,9,3, 4,12,5, 4,18,6, 8,0,9, 8,6,10, 8,7,11, 8,9,12, 8,16,13, 8,17,14, 8,18,15, 17,9,18
; 겹모음 (앞 중성, 뒤 중성, 결과)
hk_cv:          db      8,0,9, 8,1,10, 8,20,11, 13,4,14, 13,5,15, 13,20,16, 18,20,19
