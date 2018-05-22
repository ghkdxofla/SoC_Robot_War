1. 프로젝트에 VFW 추가
- 속성 -> 링커 -> 입력 -> 추가 종속성에 vfw32.lib 추가함

# const char* 형식의 인수가 LPCWSTR 형식의 매개 변수와 호환되지 않습니다
=> 유니코드 라이브러리 기반 프로젝트로 생성하면 그렇다고함
=> 프로젝트 속성 " 유니코드라이브러리 사용"에 가면 옵션에서 사용하지 않은 을 선택하라고 함
=> 유니코드 라이브러리를 사용하려면
	TCHAR szClassName = _T("Hello World") 로 _T 매크로 사용


# git에 올릴 때 .opendb가 ignore 안됐다
=> .ignore 파일에 해당 확장자 추가로 무시해주기

# callback 함수 호출에 문제
=> static으로 선언하니 문제 해결

# '이미 선언되어 있습니다'라면서 hdc를 비롯한 변수 에러
=> 헤더에 선언하지 않고 직접 .cpp에 선언하니 해결
=> http://acholyte.tistory.com/entry/%EB%B9%84%EC%A0%95%EC%A0%81-%EB%A9%A4%EB%B2%84-%EC%B0%B8%EC%A1%B0%EB%8A%94-%ED%8A%B9%EC%A0%95-%EA%B0%9C%EC%B2%B4%EC%97%90-%EC%83%81%EB%8C%80%EC%A0%81%EC%9D%B4%EC%96%B4%EC%95%BC-%ED%95%A9%EB%8B%88%EB%8B%A4

# 변환 후 영상이 중첩되고 어쩔 때는 lp->lpData 배열의 overflow가 일어나던데...?
=> 웹캠의 색상 정보가 YUY2로 되어있음 (4개로 이루어진 색데이터, 1픽셀의 3개와 그 옆 픽셀 3개의 데이터)
=> YUY2 -> RGB 변환 함수 써서 변환함
=> http://blog.daum.net/odega/32

# 영상 변환 후 뒤집힘...?
=> 해결

# Image sharpening?
=> 아직...ㅎ

# 영상 처리 속도가 너무 느린데...
=> 병렬처리 예정