f1 = open("/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_1.txt")
f2 = open("/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_2.txt")

i = 0
try:
    while(i<51):
        i += 1
        line1 = f1.readline().rstrip("\n")
        line2 = f2.readline().rstrip("\n")
        print(len(line1), len(line2))
        if(not line1 == line2):
            print("Not equal!!!")
            # print(line1[:100], line2[:100])
            for j in range(100, 10000, 25):
                if (line1[:j]!=line2[:j]):
                    print(line1[450:1000])
                    print(line2[450:1000])
                    exit()

        #     exit()
except Exception as e:
    print(e)
