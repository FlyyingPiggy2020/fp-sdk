fs = 2300;      
fc = 50;        
order = 1;      

% 设计低通滤波器
[b, a] = butter(order, fc/(fs/2), 'low');

% 生成测试信号 (注意：10Hz信号在10Hz截止频率处会被衰减3dB)
t = 0:1/fs:1-1/fs;
data = sin(2*pi*10*t) + 0.5*sin(2*pi*500*t);

% 应用滤波器
filtered_data = filter(b, a, data);

% 打印精确系数
fprintf('一阶巴特沃斯滤波器浮点系数:\n');
fprintf('b = [%f, %f]\n', b(1), b(2));
fprintf('a = [%f, %f]\n', a(1), a(2));
fprintf('一阶巴特沃斯滤波器系数:\n');
fprintf('b = [ %s ]\n', sprintf('%d ', b * 16384));
fprintf('a = [ %s ]\n', sprintf('%d ', a * 16384));
% 计算频率响应
nfft = 1024;
[H, w] = freqz(b, a, nfft, fs);
freq = w; 

% 计算相位延时
phase = unwrap(angle(H));
% 避免除以0，并修正DC点的延时计算
phase_delay_sec = -phase ./ (2*pi*freq + eps); 
% 对于一阶滤波器，0Hz处的延时可以用极小频率处的值近似
phase_delay_sec(1) = phase_delay_sec(2); 

% 绘图
figure('Name', '分析结果', 'Color', 'w');

% 1. 时域对比
subplot(3,1,1);
plot(t, data, 'Color', [0.7 0.7 0.7], 'DisplayName', '原始信号'); hold on;
plot(t, filtered_data, 'r', 'LineWidth', 1.5, 'DisplayName', '滤波后');
xlim([0 0.5]); % 只看前0.5秒
xlabel('时间 (s)'); ylabel('幅值');
title('时域信号对比 (10Hz + 500Hz)');
legend; grid on;

% 2. 幅频响应 (dB)
subplot(3,1,2);
plot(freq, 20*log10(abs(H)), 'b', 'LineWidth', 1);
xlabel('频率 (Hz)'); ylabel('幅度 (dB)');
title('幅频响应');
xlim([0 fc*5]); grid on;
line([fc fc], [-20 5], 'Color', 'r', 'LineStyle', '--'); % 标注fc

% 3. 相位延时 (ms)
subplot(3,1,3);
plot(freq, phase_delay_sec * 1000, 'g', 'LineWidth', 1);
xlabel('频率 (Hz)'); ylabel('延时 (ms)');
title('相位延时');
xlim([0 fc*5]); grid on;
line([fc fc], [0 max(phase_delay_sec*1000)], 'Color', 'r', 'LineStyle', '--');

% 输出关键点
idx = find(freq >= fc, 1);
fprintf('\n分析结果:\n');
fprintf('截止频率 %.1f Hz 处的衰减: %.2f dB\n', fc, 20*log10(abs(H(idx))));
fprintf('截止频率 %.1f Hz 处的相位延时: %.2f ms\n', fc, phase_delay_sec(idx)*1000);