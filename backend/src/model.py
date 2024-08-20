import sys
import requests

# 替换为你实际的 API URL 和 API key
API_URL = "https://api.example.com/v1/generate"
API_KEY = "your_api_key_here"

def generate_from_llama(input_text):
    # 构建请求头和请求体
    headers = {
        "Authorization": f"Bearer {API_KEY}",
        "Content-Type": "application/json"
    }
    data = {
        "prompt": input_text,
        "max_tokens": 100,  # 可根据需要调整生成的文本长度
    }

    # 发送 POST 请求到 API
    response = requests.post(API_URL, headers=headers, json=data)

    # 检查请求是否成功
    if response.status_code == 200:
        # 解析返回的 JSON 数据
        result = response.json()
        return result.get("text", "Error: No output returned from the model.")
    else:
        return f"Error: API request failed with status code {response.status_code}."

def main(input_text):
    output = generate_from_llama(input_text)
    print(output)  # 将输出打印出来供 C 程序读取

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: No input provided.")
        sys.exit(1)
    input_text = sys.argv[1]
    main(input_text)
