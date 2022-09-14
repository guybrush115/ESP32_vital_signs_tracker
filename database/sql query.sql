## Average HR by hours ago

select round(((julianday(current_timestamp) - julianday(timestamp))*24)-0.5) as hours_ago,avg(HR) 
 from vital_signs_log
 group by hours_ago;