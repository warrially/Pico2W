package main

import (
	"fmt"
	"image"
	"image/color"
	"net/http"
	"os"
	"time"

	"golang.org/x/image/bmp"
)

const (
	BASE_URL = "http://192.168.1.104"

	// 每次按键后的间隔，避免发送过快。
	KEY_DELAY = 200 * time.Millisecond
)

func main() {
	strInputFile := "input.bmp"

	pFile, err := os.Open(strInputFile)
	if err != nil {
		panic(err)
	}
	defer pFile.Close()

	pImage, err := bmp.Decode(pFile)
	if err != nil {
		panic(err)
	}

	pBounds := pImage.Bounds()

	nWidth := pBounds.Dx()
	nHeight := pBounds.Dy()

	fmt.Printf("图片尺寸: %d x %d\n", nWidth, nHeight)
	fmt.Println("开始绘制...")

	for nY := 0; nY < nHeight; nY++ {
		fmt.Printf("\n===== 第 %d / %d 行 =====\n", nY+1, nHeight)

		// 第1、3、5...行：从左向右。
		if nY%2 == 0 {
			err = drawRowLeftToRight(pImage, nY)
			if err != nil {
				panic(err)
			}
		} else {
			// 第2、4、6...行：从右向左。
			err = drawRowRightToLeft(pImage, nY)
			if err != nil {
				panic(err)
			}
		}

		// 最后一行不再向下。
		if nY < nHeight-1 {
			nX := getRowEndX(nWidth, nY)

			fmt.Printf("移动: (%d, %d) -> (%d, %d)  DOWN\n", nX, nY, nX, nY+1)

			err = pressKey("/api/down/click")
			if err != nil {
				panic(err)
			}
		}
	}

	fmt.Println("\n绘制完成")
}

// drawRowLeftToRight
// 从左向右绘制一行。
func drawRowLeftToRight(pImage image.Image, nY int) error {
	pBounds := pImage.Bounds()
	nWidth := pBounds.Dx()

	for nX := 0; nX < nWidth; nX++ {
		err := drawPixel(pImage, nX, nY)
		if err != nil {
			return err
		}

		// 最后一个像素不再向右。
		if nX < nWidth-1 {
			fmt.Printf("移动: (%d, %d) -> (%d, %d)  RIGHT\n", nX, nY, nX+1, nY)

			err = pressKey("/api/right/click")
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// drawRowRightToLeft
// 从右向左绘制一行。
func drawRowRightToLeft(pImage image.Image, nY int) error {
	pBounds := pImage.Bounds()
	nWidth := pBounds.Dx()

	for nX := nWidth - 1; nX >= 0; nX-- {
		err := drawPixel(pImage, nX, nY)
		if err != nil {
			return err
		}

		// 最后一个像素不再向左。
		if nX > 0 {
			fmt.Printf("移动: (%d, %d) -> (%d, %d)  LEFT\n", nX, nY, nX-1, nY)

			err = pressKey("/api/left/click")
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// drawPixel
// 黑色按A。
// 白色按B。
func drawPixel(pImage image.Image, nX int, nY int) error {
	if isBlack(pImage, nX, nY) {
		fmt.Printf("坐标: (%d, %d)  黑色 -> A\n", nX, nY)

		return pressKey("/api/a/click")
	}

	fmt.Printf("坐标: (%d, %d)  白色 -> B\n", nX, nY)

	return pressKey("/api/b/click")
}

// isBlack
// 判断当前像素是否为黑色。
func isBlack(pImage image.Image, nX int, nY int) bool {
	pColor := color.RGBAModel.Convert(pImage.At(nX, nY)).(color.RGBA)

	nGray := (uint32(pColor.R) + uint32(pColor.G) + uint32(pColor.B)) / 3

	return nGray < 128
}

// pressKey
// 模拟一次按键点击。
func pressKey(strAPI string) error {
	err := sendRequest(strAPI)
	if err != nil {
		return err
	}

	time.Sleep(KEY_DELAY)

	return nil
}

// sendRequest
// 发送HTTP控制请求。
func sendRequest(strAPI string) error {
	strURL := BASE_URL + strAPI

	pRsp, err := http.Get(strURL)
	if err != nil {
		return fmt.Errorf("请求失败 %s: %w", strURL, err)
	}
	defer pRsp.Body.Close()

	if pRsp.StatusCode != http.StatusOK {
		return fmt.Errorf("请求失败 %s, status=%d", strURL, pRsp.StatusCode)
	}

	return nil
}

// getRowEndX
// 获取当前行结束时的X坐标。
func getRowEndX(nWidth int, nY int) int {
	if nY%2 == 0 {
		return nWidth - 1
	}

	return 0
}
