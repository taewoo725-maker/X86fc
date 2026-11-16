; 만든이: KimTaeWoo
; X86FC 편집기 EUC-KR 변환 보조 (오버레이 11·12 가 같이 씀) - SPDX-License-Identifier: GPL-3.0-or-later
; 변환 표 = OVL 안 글꼴 뒤 EUC-KR 표 (ov_viewer 와 같음): 체크점 37 x 2 바이트 + 차이 2350 바이트.
;   순번 n = (첫 바이트 - B0h) x 94 + (둘째 - A1h) (KS X 1001 한글 2350자),  idx(n) = 체크점[n>>6] + 차이[(n AND NOT 63)+1 .. n] 의 합,  코드 = AC00h + idx
; 표는 글 쪽의 끝 TBLA(= EDBUF + 13960) 에 읽음 (그 쪽 안이라 별도 쪽이 필요 없음).  표를 쓰는 동안에는 되돌리기 기록 자리가 덮이므로 기록을 비울 것.
TBLA            equ     0B688h
TBLN            equ     2424
eu_n            equ     bw_n                    ; (큰 파일 방식과 같이 쓰이지 않으므로 그 변수를 빌림)

; 표 읽기 → TBLA.  글 쪽이 끼워진 상태에서.  NZ = 실패
eu_load:        ld      hl,SELFPATH
                ld      de,CMDBUF
                call    copy_z
                xor     a
                ld      (de),a
                dec     de
                dec     de
                dec     de
                ld      a,'O'
                ld      (de),a
                inc     de
                ld      a,'V'
                ld      (de),a
                inc     de
                ld      a,'L'
                ld      (de),a
                ld      de,CMDBUF
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (bw_h2),a
                ld      de,3
                ld      hl,HGB+201096-196608
                xor     a
                ld      c,F_SEEK
                call    BDOS
                or      a
                jr      nz,eu_x
                ld      a,(bw_h2)
                ld      b,a
                ld      de,TBLA
                ld      hl,TBLN
                ld      c,F_READ
                call    BDOS
                or      a
                jr      nz,eu_x
                ld      de,TBLN
                or      a
                sbc     hl,de
                ld      a,0
                jr      z,eu_x
                inc     a
eu_x:           push    af
                ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
                or      a
                ret
; HL = 순번 n (0~2349) → HL = idx
eu_idx:         ld      (eu_n),hl
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l                       ; n >> 6
                add     hl,hl
                ld      de,TBLA
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 체크점
                ld      hl,(eu_n)
                ld      a,l
                and     3Fh
                ld      b,a
                ld      a,l
                and     0C0h
                ld      l,a
                push    de
                ld      de,TBLA+74+1
                add     hl,de                   ; HL = 차이[base + 1]
                pop     de
                ld      a,b
                or      a
                jr      z,ei_d
ei_l:           ld      a,(hl)
                add     a,e
                ld      e,a
                jr      nc,ei_n
                inc     d
ei_n:           inc     hl
                djnz    ei_l
ei_d:           ex      de,hl
                ret
; HL = 찾는 idx → NC: HL = 순번 n,  C: 표에 없음.  (idx 는 순번이 늘면 늘므로 이진 탐색)
eu_find:        ld      (ed_t4),hl
                ld      hl,0
                ld      (ed_t3),hl              ; 아래
                ld      hl,2349
                ld      (ed_t2),hl              ; 위
ef_l:           ld      hl,(ed_t3)
                ld      de,(ed_t2)
                or      a
                push    hl
                sbc     hl,de
                pop     hl
                jr      z,ef_m                  ; 아래 = 위: 마지막 후보
                jr      nc,ef_no
ef_m:           add     hl,de
                srl     h
                rr      l                       ; 가운데
                ld      (ed_t1),hl
                call    eu_idx
                ld      de,(ed_t4)
                or      a
                sbc     hl,de                   ; idx(mid) - 찾는 값
                jr      z,ef_ok
                jr      c,ef_up                 ; idx(mid) < 찾는 값: 아래를 올림
                ld      hl,(ed_t1)              ; 위를 내림
                ld      a,h
                or      l
                jr      z,ef_no
                dec     hl
                ld      (ed_t2),hl
                jr      ef_l
ef_up:          ld      hl,(ed_t1)
                inc     hl
                ld      (ed_t3),hl
                jr      ef_l
ef_ok:          ld      hl,(ed_t1)
                or      a
                ret
ef_no:          scf
                ret
