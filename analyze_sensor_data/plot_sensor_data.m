filename1 = 'fsr1.csv';
filename2 = 'fsr2.csv';
filename3 = 'load_cell.csv';
filename4 = 'timeline_fsr.csv';
filename5 = 'timeline_load_cell.csv';

fsr1 = readmatrix(filename1);
fsr2 = readmatrix(filename2);
load_cell = readmatrix(filename3);
timeline_fsr = readmatrix(filename4);
timeline_load_cell = readmatrix(filename5);

create_plot(fsr1,timeline_fsr,1,'Raw Data Readings From FSR1','fsr1_plot.jpg');
create_plot(fsr2,timeline_fsr,2,'Raw Data Readings From FSR2','fsr2_plot.jpg');
create_plot(load_cell,timeline_load_cell,3,'Raw Data Readings From Load Cell','load_cell_plot.jpg');

maximal_value_fsr1 = max(fsr1);
maximal_value_fsr2 = max(fsr2);

fsr1_normalized = fsr1./maximal_value_fsr1;
fsr2_normalized = fsr2./maximal_value_fsr2;

create_plot(fsr1_normalized,timeline_fsr,4,'Raw Normalized Data Readings From FSR1','fsr1_normalized_plot.jpg');
create_plot(fsr2_normalized,timeline_fsr,5,'Raw Normalized Data Readings From FSR2','fsr2_normalized_plot.jpg');


function create_plot(data_matrix,timeline,index,label,fname)
    
    fig = figure(index);
    plot(timeline,data_matrix,'x',MarkerSize=3);
    %xlim([4900,6200]);
    xlabel('time');
    ylabel('value');
    title(label);
    saveas(fig,fname);

end