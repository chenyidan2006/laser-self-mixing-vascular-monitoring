# Health Monitor - AI API 集成指南

---

## 一、当前AI实现分析

### 1.1 当前方案

当前AI建议采用**规则匹配**方式，逻辑简单：

```javascript
// 当前实现（src/pages/index/index.vue）
generateAIAdvice(data) {
  const advices = [];
  
  if (data.heart_rate < 60) {
    advices.push('Your heart rate is relatively low...');
  } else if (data.heart_rate > 100) {
    advices.push('Your heart rate is elevated...');
  }
  
  // 血压、血氧类似规则...
  
  this.aiAdvice = advices.join(' ');
}
```

### 1.2 问题分析

| 问题 | 说明 | 影响 |
|------|------|------|
| **规则固定** | 只能处理预设的健康指标范围 | 无法应对复杂情况 |
| **缺乏深度分析** | 只能给出简单建议 | 不能提供个性化分析 |
| **无上下文记忆** | 每次独立判断 | 无法追踪健康趋势变化 |
| **不支持多维度分析** | 指标间无关联分析 | 无法发现潜在健康风险 |

### 1.3 改进目标

```
当前：简单规则匹配 → 改进后：外部AI深度分析

改进内容：
├── 1. 多指标综合分析（心率+血压+血氧综合判断）
├── 2. 历史数据趋势分析（对比历史数据）
├── 3. 个性化健康建议（结合用户年龄、性别、体重等）
├── 4. 健康风险预警（识别潜在健康问题）
└── 5. 自然语言交互（支持用户提问）
```

---

## 二、外部AI API接入方案

### 2.1 支持的AI服务商

| 服务商 | API类型 | 推荐场景 | 参考文档 |
|--------|---------|----------|----------|
| **DeepSeek** | 通用大模型 | 中文医疗健康领域 | https://platform.deepseek.com/ |
| **OpenAI** | 通用大模型 | 英文场景 | https://platform.openai.com/docs/ |
| **阿里云通义千问** | 通用大模型 | 国内稳定服务 | https://help.aliyun.com/document_detail/611479.html |
| **腾讯混元大模型** | 通用大模型 | 国内稳定服务 | https://cloud.tencent.com/document/api/17296 |
| **百度文心一言** | 通用大模型 | 国内稳定服务 | https://cloud.baidu.com/doc/WENXINWORKSHOP/index.html |

### 2.2 接入架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        前端 (uni-app)                          │
│  ┌─────────────┐                                              │
│  │  AI建议展示  │◄─────────────────────────────────────────────│
│  └─────────────┘                      │                        │
└───────────────────────────────────────│────────────────────────┘
                                        │ HTTP POST
                                        ▼
┌─────────────────────────────────────────────────────────────────┐
│                    后端服务 (Node.js)                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              /api/ai/health-advice                      │   │
│  │  1. 收集健康数据                                        │   │
│  │  2. 构建Prompt                                        │   │
│  │  3. 调用外部AI API                                     │   │
│  │  4. 解析返回结果                                       │   │
│  └─────────────────────────────────────────────────────────┘   │
│                              │                                │
│                              ▼                                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              外部AI API (DeepSeek等)                     │   │
│  │  POST https://api.deepseek.com/v1/chat/completions     │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 三、详细接入步骤

### 3.1 注册AI服务

**以DeepSeek为例：**

1. 访问 https://platform.deepseek.com/
2. 注册账号并登录
3. 创建API Key（在控制台 → API Keys）
4. 查看余额和调用额度

### 3.2 创建后端AI服务模块

**步骤1：创建AI服务工具**

```javascript
// health-backend/utils/aiService.js
const axios = require('axios');

// AI服务商配置
const AI_CONFIG = {
  provider: process.env.AI_PROVIDER || 'deepseek',
  apiKey: process.env.AI_API_KEY || '',
  model: process.env.AI_MODEL || 'deepseek-chat',
  baseUrl: process.env.AI_BASE_URL || 'https://api.deepseek.com/v1',
  timeout: parseInt(process.env.AI_TIMEOUT) || 30000
};

/**
 * 构建健康分析Prompt
 * @param {Object} healthData - 健康数据
 * @param {Object} userInfo - 用户信息（可选）
 * @param {Array} historyData - 历史数据（可选）
 */
function buildHealthPrompt(healthData, userInfo = {}, historyData = []) {
  let prompt = `你是一位专业的健康顾问。请基于以下健康数据为用户提供专业的分析和建议。

【健康数据】
- 心率: ${healthData.heart_rate || '--'} bpm
- 收缩压: ${healthData.systolic_bp || '--'} mmHg
- 舒张压: ${healthData.diastolic_bp || '--'} mmHg
- 血氧饱和度: ${healthData.spo2 || '--'} %
- 测量时间: ${healthData.measureTime || '未知'}

【用户信息】
- 性别: ${userInfo.gender === 1 ? '男' : userInfo.gender === 2 ? '女' : '未知'}
- 年龄: ${userInfo.birthday ? calculateAge(userInfo.birthday) : '未知'} 岁
- 身高: ${userInfo.height || '--'} cm
- 体重: ${userInfo.weight || '--'} kg
- BMI: ${userInfo.height && userInfo.weight ? 
  (userInfo.weight / Math.pow(userInfo.height / 100, 2)).toFixed(1) : '--'}

`;

  if (historyData && historyData.length > 0) {
    prompt += `【最近7天趋势】
${historyData.slice(-7).map((item, idx) => 
  `${idx + 1}. 心率: ${item.heart_rate} bpm | 血压: ${item.systolic_bp}/${item.diastolic_bp} mmHg | 血氧: ${item.spo2}%`
).join('\n')}

`;
  }

  prompt += `请按照以下结构输出分析报告：

## 1. 指标分析
- 逐一分析各项指标是否在正常范围
- 指出异常指标及其可能原因

## 2. 综合评估
- 基于所有指标给出整体健康状况评估
- 识别潜在风险（如有）

## 3. 健康建议
- 针对异常指标提供具体改善建议
- 生活方式调整建议（运动、饮食、作息等）

## 4. 就医提示
- 如果存在需要关注的健康问题，请明确建议就医检查

请用中文输出，语言通俗易懂，避免专业术语过多。
`;

  return prompt;
}

/**
 * 计算年龄
 */
function calculateAge(birthday) {
  try {
    const birthDate = new Date(birthday);
    const today = new Date();
    let age = today.getFullYear() - birthDate.getFullYear();
    const monthDiff = today.getMonth() - birthDate.getMonth();
    if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birthDate.getDate())) {
      age--;
    }
    return age;
  } catch {
    return '未知';
  }
}

/**
 * 调用DeepSeek API
 */
async function callDeepSeek(prompt) {
  try {
    const response = await axios.post(
      `${AI_CONFIG.baseUrl}/chat/completions`,
      {
        model: AI_CONFIG.model,
        messages: [
          {
            role: 'system',
            content: '你是一位专业的健康顾问AI，擅长分析健康数据并提供专业建议。'
          },
          {
            role: 'user',
            content: prompt
          }
        ],
        temperature: 0.7,
        max_tokens: 2000
      },
      {
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${AI_CONFIG.apiKey}`
        },
        timeout: AI_CONFIG.timeout
      }
    );

    return response.data.choices[0]?.message?.content || '';
  } catch (error) {
    console.error('DeepSeek API调用失败:', error.message);
    throw new Error(`AI服务调用失败: ${error.message}`);
  }
}

/**
 * 调用OpenAI API
 */
async function callOpenAI(prompt) {
  try {
    const response = await axios.post(
      'https://api.openai.com/v1/chat/completions',
      {
        model: 'gpt-3.5-turbo',
        messages: [
          {
            role: 'system',
            content: '你是一位专业的健康顾问AI。'
          },
          {
            role: 'user',
            content: prompt
          }
        ],
        temperature: 0.7
      },
      {
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${AI_CONFIG.apiKey}`
        },
        timeout: AI_CONFIG.timeout
      }
    );

    return response.data.choices[0]?.message?.content || '';
  } catch (error) {
    console.error('OpenAI API调用失败:', error.message);
    throw new Error(`AI服务调用失败: ${error.message}`);
  }
}

/**
 * 调用阿里云通义千问API
 */
async function callQwen(prompt) {
  try {
    // 需要安装 @alicloud/darabonba-core 和 @alicloud/qianwen-api
    // 参考：https://help.aliyun.com/document_detail/611479.html
    const response = await axios.post(
      'https://dashscope.aliyuncs.com/api/text/v1/generation',
      {
        model: 'qwen-turbo',
        prompt: prompt,
        max_tokens: 2000
      },
      {
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${AI_CONFIG.apiKey}`
        },
        timeout: AI_CONFIG.timeout
      }
    );

    return response.data.output?.text || '';
  } catch (error) {
    console.error('Qwen API调用失败:', error.message);
    throw new Error(`AI服务调用失败: ${error.message}`);
  }
}

/**
 * 获取AI健康建议
 * @param {Object} healthData - 健康数据
 * @param {Object} userInfo - 用户信息
 * @param {Array} historyData - 历史数据
 */
async function getHealthAdvice(healthData, userInfo = {}, historyData = []) {
  // 构建Prompt
  const prompt = buildHealthPrompt(healthData, userInfo, historyData);
  
  // 根据配置调用不同服务商
  switch (AI_CONFIG.provider.toLowerCase()) {
    case 'deepseek':
      return await callDeepSeek(prompt);
    case 'openai':
      return await callOpenAI(prompt);
    case 'qwen':
      return await callQwen(prompt);
    default:
      throw new Error(`不支持的AI服务商: ${AI_CONFIG.provider}`);
  }
}

module.exports = {
  getHealthAdvice,
  buildHealthPrompt,
  AI_CONFIG
};
```

**步骤2：添加环境变量配置**

```env
# .env 文件添加以下配置

# AI服务配置
AI_PROVIDER=deepseek           # deepseek, openai, qwen
AI_API_KEY=your-api-key-here   # 你的API Key
AI_MODEL=deepseek-chat         # 模型名称
AI_BASE_URL=https://api.deepseek.com/v1
AI_TIMEOUT=30000
```

**步骤3：创建AI路由**

```javascript
// health-backend/routes/ai.js
const express = require('express');
const router = express.Router();
const { getHealthAdvice } = require('../utils/aiService');
const authMiddleware = require('../middleware/auth');
const HealthDataModel = require('../models/HealthDataModel');

/**
 * 获取AI健康建议
 * POST /api/ai/health-advice
 */
router.post('/health-advice', authMiddleware, async (req, res) => {
  try {
    const { healthData, userInfo, includeHistory = true } = req.body;
    
    // 获取历史数据
    let historyData = [];
    if (includeHistory) {
      const records = await HealthDataModel.getRecentRecords(req.user.id, 7);
      historyData = records.map(r => ({
        heart_rate: r.heart_rate,
        systolic_bp: r.systolic_bp,
        diastolic_bp: r.diastolic_bp,
        spo2: r.spo2,
        measure_time: r.measure_time
      }));
    }
    
    // 调用AI服务
    const advice = await getHealthAdvice(healthData, userInfo, historyData);
    
    res.json({
      errcode: 0,
      msg: 'success',
      data: {
        advice: advice,
        timestamp: new Date().toISOString()
      }
    });
  } catch (error) {
    console.error('AI建议生成失败:', error.message);
    res.json({
      errcode: -1,
      msg: `AI服务调用失败: ${error.message}`
    });
  }
});

/**
 * 获取模拟AI建议（备用方案）
 */
router.post('/health-advice/mock', (req, res) => {
  const { healthData } = req.body;
  const advices = [];
  
  if (healthData.heart_rate) {
    if (healthData.heart_rate < 60) {
      advices.push('心率偏低，建议进行适度有氧运动，如快走、慢跑，以提升心肺功能。');
    } else if (healthData.heart_rate > 100) {
      advices.push('心率偏高，建议进行深呼吸练习，保持情绪稳定，避免过度紧张。');
    } else {
      advices.push('心率正常，继续保持规律运动习惯！');
    }
  }
  
  if (healthData.systolic_bp) {
    if (healthData.systolic_bp > 140) {
      advices.push('血压偏高，建议减少盐分摄入，保持低盐饮食，定期监测血压变化。');
    } else if (healthData.systolic_bp < 90) {
      advices.push('血压偏低，建议保证充足饮水，避免突然站立，适当增加蛋白质摄入。');
    } else {
      advices.push('血压处于正常范围，继续保持健康生活方式！');
    }
  }
  
  if (healthData.spo2) {
    if (healthData.spo2 < 95) {
      advices.push('血氧饱和度偏低，建议到空气流通处深呼吸，如有持续不适请及时就医。');
    } else {
      advices.push('血氧饱和度良好，继续保持！');
    }
  }
  
  // 综合建议
  advices.push('\n**综合建议**：保持规律作息，保证充足睡眠（7-8小时/天），饮食均衡，每周进行3-5次运动。如有任何不适，请及时咨询医生。');
  
  res.json({
    errcode: 0,
    msg: 'success',
    data: {
      advice: advices.join('\n\n'),
      timestamp: new Date().toISOString(),
      mock: true
    }
  });
});

module.exports = router;
```

**步骤4：更新data-server.js添加AI路由**

```javascript
// data-server.js
const aiRouter = require('./routes/ai');
app.use('/api/ai', aiRouter);
```

### 3.3 更新前端调用

**步骤1：更新API配置**

```javascript
// src/config/api.js
export const API_CONFIG = {
  // ... 其他配置
  data: {
    baseUrl: 'http://localhost:3002',
    // ... 其他端点
    aiAdvice: '/api/ai/health-advice',
    aiAdviceMock: '/api/ai/health-advice/mock'
  }
};
```

**步骤2：更新前端AI建议获取逻辑**

```javascript
// src/pages/index/index.vue
async generateAIAdvice(data) {
  try {
    const user = uni.getStorageSync('user') || {};
    
    const response = await uni.request({
      url: getDataUrl('aiAdvice'),
      method: 'POST',
      header: {
        Authorization: 'Bearer ' + uni.getStorageSync('token'),
        'Content-Type': 'application/json'
      },
      data: {
        healthData: {
          heart_rate: data.heart_rate,
          systolic_bp: data.systolic_bp,
          diastolic_bp: data.diastolic_bp,
          spo2: data.spo2,
          measureTime: new Date().toISOString()
        },
        userInfo: {
          gender: user.gender,
          birthday: user.birthday,
          height: user.height,
          weight: user.weight
        },
        includeHistory: true
      }
    });
    
    if (response.data.errcode === 0) {
      this.aiAdvice = response.data.data.advice;
    } else {
      // 使用本地模拟建议作为降级方案
      this.generateLocalAdvice(data);
    }
  } catch (error) {
    console.error('AI建议获取失败:', error);
    // 使用本地模拟建议
    this.generateLocalAdvice(data);
  }
},

generateLocalAdvice(data) {
  const advices = [];
  
  if (data.heart_rate) {
    if (data.heart_rate < 60) {
      advices.push('心率偏低，建议进行适度有氧运动。');
    } else if (data.heart_rate > 100) {
      advices.push('心率偏高，建议深呼吸放松。');
    } else {
      advices.push('心率正常，继续保持！');
    }
  }
  
  // ... 其他指标处理
  
  this.aiAdvice = advices.join(' ');
}
```

---

## 四、不同服务商配置说明

### 4.1 DeepSeek（推荐）

```env
AI_PROVIDER=deepseek
AI_API_KEY=sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
AI_MODEL=deepseek-chat
AI_BASE_URL=https://api.deepseek.com/v1
```

**申请地址**：https://platform.deepseek.com/

### 4.2 OpenAI

```env
AI_PROVIDER=openai
AI_API_KEY=sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
AI_MODEL=gpt-3.5-turbo
AI_BASE_URL=https://api.openai.com/v1
```

**申请地址**：https://platform.openai.com/

### 4.3 阿里云通义千问

```env
AI_PROVIDER=qwen
AI_API_KEY=your-dashscope-api-key
AI_MODEL=qwen-turbo
AI_BASE_URL=https://dashscope.aliyuncs.com/api/text/v1
```

**申请地址**：https://dashscope.console.aliyun.com/

---

## 五、API调用示例

### 5.1 测试AI建议API

```bash
curl -X POST http://localhost:3002/api/ai/health-advice \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer your-jwt-token" \
  -d '{
    "healthData": {
      "heart_rate": 72,
      "systolic_bp": 120,
      "diastolic_bp": 80,
      "spo2": 98,
      "measureTime": "2026-05-07T10:30:00.000Z"
    },
    "userInfo": {
      "gender": 1,
      "birthday": "1990-01-01",
      "height": 175,
      "weight": 70
    },
    "includeHistory": true
  }'
```

### 5.2 测试模拟API（无需AI Key）

```bash
curl -X POST http://localhost:3002/api/ai/health-advice/mock \
  -H "Content-Type: application/json" \
  -d '{
    "healthData": {
      "heart_rate": 72,
      "systolic_bp": 120,
      "diastolic_bp": 80,
      "spo2": 98
    }
  }'
```

---

## 六、安全性注意事项

### 6.1 API Key保护

| 措施 | 说明 |
|------|------|
| **环境变量存储** | 不要硬编码API Key，使用.env文件 |
| **权限限制** | 给API Key设置最小权限范围 |
| **定期轮换** | 定期更换API Key |
| **网络限制** | 在服务商后台设置IP白名单 |

### 6.2 数据安全

| 措施 | 说明 |
|------|------|
| **数据脱敏** | 传输前移除敏感信息 |
| **HTTPS加密** | 使用HTTPS传输数据 |
| **数据最小化** | 只传输必要的健康数据 |
| **日志脱敏** | 日志中不记录完整健康数据 |

### 6.3 成本控制

| 措施 | 说明 |
|------|------|
| **设置预算** | 在服务商后台设置消费上限 |
| **缓存策略** | 相同数据短时间内返回缓存结果 |
| **降级方案** | AI服务不可用时使用本地规则 |
| **请求节流** | 限制单位时间内的请求次数 |

---

## 七、部署检查清单

```
部署前检查：
├── ✅ AI服务商账号已注册
├── ✅ API Key已获取
├── ✅ .env文件已配置AI相关环境变量
├── ✅ aiService.js已创建
├── ✅ ai.js路由已添加
├── ✅ data-server.js已挂载AI路由
├── ✅ 前端已更新API调用
├── ✅ 网络可达性测试（测试API调用）
└── ✅ 错误降级逻辑已实现
```

---

## 八、后续优化建议

### 8.1 功能扩展

```
下一步可以添加：
├── 1. 用户对话历史存储
│   └── 支持多轮对话，让AI了解用户健康变化
├── 2. 健康报告生成
│   └── 定期生成周/月健康报告
├── 3. 个性化健康计划
│   └── 根据用户数据生成运动/饮食计划
├── 4. 健康预警系统
│   └── 异常指标自动提醒
└── 5. 多语言支持
    └── 支持中英双语健康建议
```

### 8.2 性能优化

```
性能优化方向：
├── Redis缓存AI结果（相同数据5分钟内返回缓存）
├── 异步处理（非关键路径使用消息队列）
├── 请求合并（短时间内多次请求合并为一次）
└── 本地规则快速响应（简单情况不调用AI）
```

---

**文档版本**：v1.0  
**最后更新**：2026-05-07  
**相关文档**：[DEBUG_GUIDE.md](DEBUG_GUIDE.md) | [USAGE_GUIDE.md](USAGE_GUIDE.md)
