; 목록(ov_arc.inc = 오버레이 6)과 풀기(ov_arcx.inc = 오버레이 13)가 같이 쓰는 머리 해석·표시 비트 루틴
; ---- 파일 읽기: DE:HL = 위치, BC = 바이트 수 → SECBUF, HL = 읽은 바이트 수 (오류·끝이면 0)
ar_rd:          push    bc
                ld      a,(ar_h)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                pop     hl
                ld      a,(ar_h)
                ld      b,a
                ld      de,SECBUF
                ld      c,F_READ
                call    BDOS
                or      a
                ret     z
                ld      hl,0
                ret
ar_rdp:         ld      hl,(ar_pos)             ; ar_pos 에서 BC 바이트
                ld      de,(ar_pos+2)
                jr      ar_rd
; ar_pos += DE:HL
ar_add:         push    hl
                ld      hl,(ar_pos)
                pop     bc
                add     hl,bc
                ld      (ar_pos),hl
                ld      hl,(ar_pos+2)
                adc     hl,de
                ld      (ar_pos+2),hl
                ret
; ---- DE 번째 항목 해석 → AR_REC
ar_rec:         ex      de,hl
                add     hl,hl
                add     hl,hl
                ld      de,AR_OFF
                add     hl,de
                ld      de,ar_pos
                ld      bc,4
                ldir
                ld      hl,AR_REC               ; 비움
                ld      de,AR_REC+1
                ld      bc,95
                ld      (hl),0
                ldir
                ld      hl,AR_REC+R_NAME
                ld      (ar_np),hl
                ld      bc,300
                call    ar_rdp                  ; 머리 한 덩어리
                ld      a,(ar_fmt)
                cp      2
                jp      z,rc_zip
                cp      3
                jp      z,rc_arj
                ld      hl,SECBUF+3             ; --- LZH: 방식 "lh5"
                ld      de,AR_REC+R_METH
                ld      bc,3
                ldir
                ld      a,(SECBUF+5)
                ld      (AR_REC+R_MCH),a
                ld      a,(SECBUF+20)           ; 자료 위치 = 머리 위치 + 머리 크기 (수준 2 = 16비트, 0·1 = 바이트 + 2)
                cp      2
                ld      hl,(SECBUF)
                jr      z,rl_h
                ld      h,0
                inc     hl
                inc     hl
rl_h:           ex      de,hl
                ld      hl,(ar_pos)
                add     hl,de
                ld      (AR_REC+R_DOFF),hl
                ld      hl,(ar_pos+2)
                ld      de,0
                adc     hl,de
                ld      (AR_REC+R_DOFF+2),hl
                ld      hl,(SECBUF+7)
                ld      (AR_REC+R_CSZ),hl
                ld      hl,(SECBUF+9)
                ld      (AR_REC+R_CSZ+2),hl
                ld      hl,(SECBUF+11)
                ld      (AR_REC+R_OSZ),hl
                ld      hl,(SECBUF+13)
                ld      (AR_REC+R_OSZ+2),hl
                ld      hl,0
                ld      (ar_et),hl
                ld      (ar_n1l),hl
                ld      a,(SECBUF+20)
                cp      2
                jr      z,rc_l2
                ld      hl,(SECBUF+17)          ; 수준 0·1: DOS 날짜·시각
                ld      (AR_REC+R_DATE),hl
                ld      hl,(SECBUF+15)
                ld      (AR_REC+R_TIME),hl
                ld      a,(SECBUF+21)           ; 이름 길이
                ld      c,a
                ld      b,0
                ld      (ar_n1l),bc
                ld      hl,SECBUF+22
                ld      (ar_n1p),hl
                add     hl,bc                   ; 이름 뒤의 CRC16
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      (AR_REC+R_CRC),de
                ld      a,(SECBUF+20)
                or      a
                jr      nz,rc_l1
                ld      hl,(ar_n1p)             ; 수준 0: 이름은 머리 안
                ld      bc,(ar_n1l)
                jp      rc_nm
rc_l1:          ld      a,(SECBUF)              ; 수준 1: 확장 머리 사슬은 머리 끝 2 바이트(다음 크기)부터
                ld      e,a
                ld      d,0
                ld      hl,SECBUF
                add     hl,de
                call    rc_chain
                ld      hl,(AR_REC+R_DOFF)      ; 자료는 확장 머리들 뒤, 압축 크기에서 확장 머리 몫을 뺌
                ld      de,(ar_et)
                add     hl,de
                ld      (AR_REC+R_DOFF),hl
                ld      hl,(AR_REC+R_DOFF+2)
                ld      de,0
                adc     hl,de
                ld      (AR_REC+R_DOFF+2),hl
                ld      hl,(AR_REC+R_CSZ)
                ld      de,(ar_et)
                or      a
                sbc     hl,de
                ld      (AR_REC+R_CSZ),hl
                ld      hl,(AR_REC+R_CSZ+2)
                ld      de,0
                sbc     hl,de
                ld      (AR_REC+R_CSZ+2),hl
                ret
rc_l2:          ld      hl,(SECBUF+21)
                ld      (AR_REC+R_CRC),hl
                ld      hl,(SECBUF+15)          ; 수준 2: 유닉스 시각 → DOS 날짜
                ld      de,(SECBUF+17)
                call    ar_unix
                ld      hl,SECBUF+24
; 확장 머리 사슬: HL = [다음 크기(2)] 자리.  각 머리 = [종류(1) 자료...] + 다음 크기(2).  종류 1 = 파일 이름, 2 = 폴더 이름(구분자 FFh)
;  폴더 이름을 먼저 R_NAME 에 넣고, 끝에서 파일 이름(ar_n1)을 덧붙임.  ar_et += 머리 크기들
rc_chain:       ld      c,(hl)
                inc     hl
                ld      b,(hl)
                inc     hl
                ld      a,b
                or      c
                jr      z,rc_ce
                push    hl
                ld      hl,(ar_et)
                add     hl,bc
                ld      (ar_et),hl
                pop     hl
                push    hl                      ; 다음 사슬 = 종류 자리 + 크기 - 2
                add     hl,bc
                dec     hl
                dec     hl
                ld      (ar_nx),hl
                pop     hl
                ld      a,(hl)
                inc     hl
                dec     bc
                dec     bc
                dec     bc                      ; 자료 길이
                cp      1
                jr      nz,rc_t2
                ld      (ar_n1l),bc
                ld      (ar_n1p),hl
                jr      rc_xn
rc_t2:          cp      2
                jr      nz,rc_xn
                ld      de,(ar_np)
                push    de
                call    rc_nm
                pop     hl                      ; 복사한 폴더 이름의 FFh → '/'
rc_xf:          ld      de,(ar_np)
                or      a
                sbc     hl,de
                add     hl,de
                jr      nc,rc_xn
                ld      a,(hl)
                cp      0FFh
                jr      nz,rc_xg
                ld      (hl),'/'
rc_xg:          inc     hl
                jr      rc_xf
rc_xn:          ld      hl,(ar_nx)
                jr      rc_chain
rc_ce:          ld      hl,(ar_n1p)
                ld      bc,(ar_n1l)
; HL = 이름 글, BC = 길이 → R_NAME 의 ar_np 자리에 덧붙임 (전체 63 까지)
rc_nm:          push    hl
                ld      hl,(ar_np)
                ld      de,AR_REC
                or      a
                sbc     hl,de
                ld      a,63
                sub     l
                ld      e,a
                ld      d,0
                pop     hl
                ld      a,b
                or      a
                jr      nz,rc_n63
                ld      a,c
                cp      e
                jr      c,rc_nc
rc_n63:         ld      b,0
                ld      c,e
rc_nc:          ld      a,b
                or      c
                ret     z
                ld      de,(ar_np)
                ldir
                ld      (ar_np),de
                xor     a
                ld      (de),a
                ret
rc_zip:         ld      hl,(SECBUF+16)          ; CRC32 (+16), 시각 (+12), 지역 머리 위치 (+42)
                ld      (AR_REC+R_CRC),hl
                ld      hl,(SECBUF+18)
                ld      (AR_REC+R_CRC+2),hl
                ld      hl,(SECBUF+12)
                ld      (AR_REC+R_TIME),hl
                ld      hl,(SECBUF+42)
                ld      (AR_REC+R_DOFF),hl
                ld      hl,(SECBUF+44)
                ld      (AR_REC+R_DOFF+2),hl
                ld      a,(SECBUF+10)           ; 방식 0 → S, 8 → Z, 그 밖 (또는 암호) = ?
                ld      b,'S'
                or      a
                jr      z,rz_mc
                ld      b,'Z'
                cp      8
                jr      z,rz_mc
                ld      b,'?'
rz_mc:          ld      a,(SECBUF+8)            ; 일반 목적 비트 0 = 암호
                and     1
                jr      z,rz_mk
                ld      b,'?'
rz_mk:          ld      a,b
                ld      (AR_REC+R_MCH),a
                ld      hl,(SECBUF+24)          ; 원래 크기 (+24), 압축 크기 (+20)
                ld      (AR_REC+R_OSZ),hl
                ld      hl,(SECBUF+26)
                ld      (AR_REC+R_OSZ+2),hl
                ld      hl,(SECBUF+20)
                ld      (AR_REC+R_CSZ),hl
                ld      hl,(SECBUF+22)
                ld      (AR_REC+R_CSZ+2),hl
                ld      hl,(SECBUF+14)
                ld      (AR_REC+R_DATE),hl
                ld      a,(SECBUF+10)           ; 방식: 0 저장, 8 deflate, 그 밖 = ?
                ld      hl,ar_tstore
                or      a
                jr      z,rz_m
                ld      hl,ar_tdefl
                cp      8
                jr      z,rz_m
                ld      hl,ar_tother
rz_m:           ld      de,AR_REC+R_METH
                call    copy_z
                ld      bc,(SECBUF+28)          ; 이름 길이 (이름은 머리 46 바이트 뒤)
                ld      hl,SECBUF+46
                call    rc_nm
                ld      hl,(ar_np)              ; 이름이 '/' 로 끝나면 폴더 항목
                dec     hl
                ld      a,(hl)
                cp      '/'
                ret     nz
                ld      a,'d'
                ld      (AR_REC+R_MCH),a
                ret
rc_arj:         ld      hl,(SECBUF+20)
                ld      (AR_REC+R_OSZ),hl
                ld      hl,(SECBUF+22)
                ld      (AR_REC+R_OSZ+2),hl
                ld      hl,(SECBUF+16)
                ld      (AR_REC+R_CSZ),hl
                ld      hl,(SECBUF+18)
                ld      (AR_REC+R_CSZ+2),hl
                ld      hl,(SECBUF+24)          ; CRC32
                ld      (AR_REC+R_CRC),hl
                ld      hl,(SECBUF+26)
                ld      (AR_REC+R_CRC+2),hl
                ld      hl,(SECBUF+12)          ; 시각 낮은 글 12~13, 날짜 14~15 (유닉스에서 만든 것은 유닉스 시각 12~15)
                ld      (AR_REC+R_TIME),hl
                ld      de,(SECBUF+14)
                ld      (AR_REC+R_DATE),de
                ld      a,(SECBUF+7)
                cp      2
                jr      nz,rj_t
                call    ar_unix
                ld      hl,0
                ld      (AR_REC+R_TIME),hl
rj_t:           ld      a,'m'
                ld      (AR_REC+R_METH),a
                ld      a,(SECBUF+9)            ; 방식 0~4
                add     a,'0'
                ld      (AR_REC+R_METH+1),a
                ld      (AR_REC+R_MCH),a
                ld      a,(SECBUF+8)            ; 암호 걸림 = 못 풂
                and     1
                ld      a,'?'
                jr      nz,rj_m
                ld      a,(SECBUF+10)           ; 파일 종류 3 = 폴더
                cp      3
                ld      a,'d'
                jr      z,rj_m
                ld      a,(AR_REC+R_MCH)
rj_m:           ld      (AR_REC+R_MCH),a
                ld      a,(SECBUF+4)            ; 이름 = 머리 시작 + 4 + 첫 머리 크기
                ld      e,a
                ld      d,0
                ld      hl,SECBUF+4
                add     hl,de
                ld      bc,63
                call    rc_nm
                ld      hl,(SECBUF+2)           ; 자료 위치 = 머리 + 8 + 머리 크기 + 확장 머리들 + 2
                ld      de,8
                add     hl,de
                ld      de,0
                call    ar_add
rj_x:           ld      bc,2
                call    ar_rdp
                ld      hl,(SECBUF)
                ld      a,h
                or      l
                jr      z,rj_e
                ld      bc,6
                add     hl,bc
                ld      de,0
                call    ar_add
                jr      rj_x
rj_e:           ld      hl,2
                ld      de,0
                call    ar_add
                ld      hl,(ar_pos)
                ld      (AR_REC+R_DOFF),hl
                ld      hl,(ar_pos+2)
                ld      (AR_REC+R_DOFF+2),hl
                ret
ar_tstore:      db      "store",0
ar_tdefl:       db      "defl",0
ar_tother:      db      "?",0

; 유닉스 시각 DE:HL → R_DATE (DOS 날짜).  일수 = (시각 >> 7) / 675 (86400 = 128 x 675), 연도·달은 차례로 빼서 구함
ar_unix:        ld      b,7
au_s:           srl     d
                rr      e
                rr      h
                rr      l
                djnz    au_s
                ld      (ar_q),hl
                ld      (ar_q+2),de
                ld      hl,0                    ; 나머지
                ld      b,32
au_l:           ld      a,(ar_q)                ; 32비트를 왼쪽으로 한 비트 밀어 넘친 비트를 나머지에 넣고 675 로 나눔
                add     a,a
                ld      (ar_q),a
                ld      a,(ar_q+1)
                rla
                ld      (ar_q+1),a
                ld      a,(ar_q+2)
                rla
                ld      (ar_q+2),a
                ld      a,(ar_q+3)
                rla
                ld      (ar_q+3),a
                adc     hl,hl
                ld      de,675
                or      a
                sbc     hl,de
                jr      nc,au_1
                add     hl,de
                jr      au_n
au_1:           ld      a,(ar_q)
                or      1
                ld      (ar_q),a
au_n:           djnz    au_l
                ld      hl,(ar_q)               ; HL = 1970-01-01 부터의 일수
                ld      bc,1970
au_y:           ld      a,c
                and     3
                ld      de,365
                jr      nz,au_y2
                inc     de
au_y2:          or      a
                sbc     hl,de
                jr      c,au_yb
                inc     bc
                jr      au_y
au_yb:          add     hl,de                   ; HL = 그 해의 0 부터 센 날, BC = 연도
                ld      (ar_y),bc
                ld      ix,ar_mt
                ld      c,1                     ; 달
au_m:           ld      a,(ix+0)
                ld      b,a
                ld      a,c
                cp      2
                jr      nz,au_m2
                ld      a,(ar_y)
                and     3
                jr      nz,au_m2
                inc     b                       ; 윤년의 2월 = 29일
au_m2:          ld      d,0
                ld      e,b
                or      a
                sbc     hl,de
                jr      c,au_md
                inc     ix
                inc     c
                jr      au_m
au_md:          add     hl,de
                inc     l                       ; 일 = 0 부터 센 날 + 1
                ld      b,l                     ; B = 일, C = 달
                ld      hl,(ar_y)
                ld      de,-1980
                add     hl,de
                jr      c,au_ok                 ; 1980 이후
                ld      hl,0
                ld      (AR_REC+R_DATE),hl
                ret
au_ok:          ld      a,l
                add     a,a                     ; (연도-1980) << 1
                ld      d,a
                ld      a,c                     ; 높은 바이트 = (연도-1980) << 1 | 달 >> 3
                srl     a
                srl     a
                srl     a
                or      d
                ld      d,a                     ; 높은 바이트
                ld      a,c
                and     7
                rrca
                rrca
                rrca                            ; (달 & 7) << 5
                or      b
                ld      e,a
                ld      (AR_REC+R_DATE),de
                ret
ar_mt:          db      31,28,31,30,31,30,31,31,30,31,30,31

; HL = 번호 → HL = 표시 바이트 번지, A = 비트
mk_addr:        ld      a,l
                and     7
                ld      b,a
                ld      a,1
                inc     b
mk_l:           dec     b
                jr      z,mk_e
                add     a,a
                jr      mk_l
mk_e:           ld      c,a
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                ld      de,AR_MARK
                add     hl,de
                ld      a,c
                ret
; 표시한 개수 → HL
ar_cnt:         ld      hl,0
                ld      (ar_t),hl
                ld      (ar_c),hl
ac_l:           ld      hl,(ar_t)
                ld      de,(ar_n)
                or      a
                sbc     hl,de
                jr      nc,ac_d
                ld      hl,(ar_t)
                call    mk_addr
                and     (hl)
                jr      z,ac_n
                ld      hl,(ar_c)
                inc     hl
                ld      (ar_c),hl
ac_n:           ld      hl,(ar_t)
                inc     hl
                ld      (ar_t),hl
                jr      ac_l
ac_d:           ld      hl,(ar_c)
                ret
