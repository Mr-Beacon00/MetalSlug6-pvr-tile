#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void split_and_write(FILE *sourceFile, FILE *target) {
    uint8_t byte;
    uint32_t sourceAddr = 0;
    uint32_t rounds = 0;
    uint32_t rounds2 = 0;
    uint32_t rounds3 = 0;
    uint8_t isHigh = 0;
    uint32_t highInc = 0x00;
    uint8_t isLow4x4 = 0;
    uint32_t high = 0x00;
    uint32_t base1 = 0x00;
    uint32_t base2 = 0x08;

    fseek(sourceFile, 0, SEEK_END);
    long fileSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);

    while (sourceAddr+1 < fileSize) {
        //printf("sourceAddr %u of %u Rounds:%u Rounds2:%u Rounds3:%u\n",sourceAddr,fileSize,rounds,rounds2,rounds3);
        if (rounds >= 8) {
            rounds=0;
            //printf("round reset!!!\n");
        }
        if (rounds2 >= 32) {
            rounds2=0;
            //printf("round2 reset!!!\n");
        }
        if (rounds3 >= 64) {
            rounds3=0;
            base1 = 0x00;
            base2 = 0x08;
            highInc = 0x00;
            //printf("round3 reset, one 16x16 completed!!!\n");
        }


        if (rounds % 2==1) { // 2x2 bottom
            high += 0x10;
            //printf("high!\n");
        }
        if (rounds % 2 == 0 && rounds != 0) { // 2x2 top and move to right(top:0px left:2px)
            high -= 0x10;
            base1 += 0x01;
            base2 += 0x01;
            //printf("reset high!\n");
        }
        if (rounds %4 == 0 && rounds !=0) { // 4x4 done (top:0px left: 2px)
            highInc += 0x20;
            base1 -= 0x2;
            base2 -= 0x2;
            //printf("after 4x4!!!\n");
        }
        if (rounds2 % 8 == 0 && rounds2 != 0) { // 4x8 done (top:0px left:4px)
            highInc -= 0x30;
            base1 += 0x1;
            base2 += 0x1;
            //printf("after 4x8!!!\n");
        }
        if (rounds2 == 16 && rounds2 !=0) { // 8x8 done (top:8px left:0px)
            highInc += 0x40;
            base1 = 0x0;
            base2 = 0x8;
            //printf("after 8x8!\n");
        }
        if (rounds3 == 32) { // 8x16 (half) done (top:0px left:16px)
            highInc -= 0x70;
            base1 += 0x1;
            base2 += 0x1;
            //printf("after 8x16!!!\n");
        }
        if (rounds3 == 48 && rounds3 !=0) { // 4 out 4
            //highInc += 0x40;
            base1 += 0x4;
            base2 += 0x4;
            //printf("eeeee!\n");
        }

        printf("base:%X %X\n",base1+high+highInc, base2+high+highInc);
        //printf("putt%X\n",high+highInc);

        // 读取源文件中的字节，并进行拆分
        fseek(sourceFile, sourceAddr, SEEK_SET);
        fread(&byte, 1, 1, sourceFile);

        // 拆分源文件0x00位置的字节
        uint8_t left_half_0 = (byte >> 4) & 0x0F;  // 取高4位
        uint8_t right_half_0 = byte & 0x0F;        // 取低4位

        // 将拆分后的字节写入目标文件
        fseek(target, base1+(high+highInc), SEEK_SET);
        fputc((right_half_0 << 4) | 0x00, target); // 右半字节放到目标0x00位置的右侧
        fseek(target, base2+(high+highInc), SEEK_SET);
        fputc((left_half_0 << 4) | 0x00, target);  // 左半字节放到目标0x08位置的右侧

        //printf("%X\n",byte);
        //printf("base1:%X base2:%X\n",base1+high, base2+high);

        // 读取源文件0x01位置的字节
        fseek(sourceFile, sourceAddr+0x01, SEEK_SET);
        fread(&byte, 1, 1, sourceFile);

        // 拆分源文件0x01位置的字节
        uint8_t left_half_1 = (byte >> 4) & 0x0F;  // 取高4位
        uint8_t right_half_1 = byte & 0x0F;        // 取低4位

        // 将拆分后的字节写入目标文件
        fseek(target, base1+(high+highInc), SEEK_SET);
        fputc((right_half_1 << 4) | (right_half_0 & 0x0F), target); // 右半字节放到目标0x00位置的左侧
        fseek(target, base2+(high+highInc), SEEK_SET);
        fputc((left_half_1 << 4) | (left_half_0 & 0x0F), target); // 左半字节放到目标0x08位置的左侧

        //printf("%X\n",byte);
        //printf("base1:%X base2:%X\n",base1+high, base2+high);

        sourceAddr+=2;
        rounds++;
        rounds2++;
        rounds3++;
        //printf("--------------------\n");

    }

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.bin> <output.bin>\n", argv[0]);
        return 1;
    }

    FILE *sourceFile = fopen(argv[1], "rb");
    if (!sourceFile) {
        perror("Error opening source file");
        return 1;
    }

    FILE *target = fopen(argv[2], "wb");
    if (!target) {
        perror("Error opening target file");
        fclose(sourceFile);
        return 1;
    }

    split_and_write(sourceFile, target);

    fclose(sourceFile);
    fclose(target);

    return 0;
}
