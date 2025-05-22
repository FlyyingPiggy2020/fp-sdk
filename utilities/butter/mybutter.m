fs = 5000;      % 采样率
fc = 10;        % 截止频率 10 Hz
order = 1;      % 一阶

% 设计低通滤波器
[b, a] = butter(order, fc/(fs/2), 'low');

% 生成测试信号（示例：包含5Hz和50Hz成分）
t = 0:1/fs:1-1/fs;
data = sin(2*pi*10*t) + 0.5*sin(2*pi*500*t);

% 应用滤波器
filtered_data = filter(b, a, data);

% 绘制结果
figure;
subplot(2,1,1);
plot(t, data);
title('原始信号');
subplot(2,1,2);
plot(t, filtered_data);
title('滤波后信号');
fprintf('一阶巴特沃斯滤波器系数:\n');
fprintf('b = [ %s ]\n', sprintf('%d ', b * 16384));
fprintf('a = [ %s ]\n', sprintf('%d ', a * 16384));