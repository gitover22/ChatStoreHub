git clone https://github.com/open-webui/open-webui.git
cd open-webui/

# Copying required .env file
cp -RPp .env.example .env

# Start frontend server
npm install
npm run dev

cd ./backend

# Optional: To install using Conda as your development environment, follow these instructions:
# Create and activate a Conda environment
conda create --name open-webui-env python=3.11
conda activate open-webui-env

# Install dependencies
pip install -r requirements.txt -U

# Start the application
bash dev.sh
