%filename = 'Example - I2C - 50kSPS.wav';
%filename = 'Example - Potentiometer + SawTooth - 5kSPS.wav';
filename = 'Example - 50Hz-Noise - 50kSPS.wav';


audioinfo (filename)

[Y, FS] = audioread (filename);

figure;
plot(Y(:,2:3));
title('First Overview (unscaled)');
grid on;

ch1_buffs = Y(:,2)*26.4;
ch2_buffs = Y(:,3)*26.4;

if length(ch1_buffs) > 50000
  figure; hold on;
  plot(ch1_buffs(1:50000));
  plot(ch2_buffs(1:50000));
  title('First 50000 Samples (scaled to V)');
  grid on;
end

vref_buffs = Y(:,1);
skips = vref_buffs(1:200:end)*32768;
figure;
plot(skips, 'LineWidth',2);
title('Skips');
grid on;
xlabel('Transfer Buffers [a 200 Samples]');
ylabel('Skips [lost buffers]');

losts = sum(skips)
recs = length(vref_buffs)/200

t_total = (recs+losts)*200/FS
lost_pers = losts/(recs+losts)*100

ns = (losts+recs)*200;
ch1 = ones(200,ns/200) * nan; % Preallocate result space
ch2 = ch1;

ch1_blocks = reshape(ch1_buffs,200,length(ch1_buffs)/200);
ch2_blocks = reshape(ch2_buffs,200,length(ch2_buffs)/200);

% Build a placement index for the blocks, insert and reshape to linear.
idx_t = cumsum(skips+1);
ch1(:,idx_t) = ch1_blocks;
ch2(:,idx_t) = ch2_blocks;
ch1 = reshape(ch1,ns,1);
ch2 = reshape(ch2,ns,1);

% Build a time vector
t = [0:ns-1]/FS; % Time in ms

figure; hold on;
plot(t,ch1, 'LineWidth',1.5);
plot(t,ch2, 'LineWidth',1.5);
title('Osci-Record (scaled and time correct)');
grid on;
xlabel('Time [s]');
ylabel('Voltage [V]');
