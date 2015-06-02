function identical = check_last_inputs(fs_new,channel_power_new,D_new,alphaDB_new,gam_new,res_Disp_new,HalfWindow_new)

    identical = 1;

    try
        load xpm_inputs.mat;
        load xpm_database.mat;
    catch
       identical = 0;
    end

    if(identical == 1)    
        if (length(fs_new) ~= length(fs))
            identical = 0;
        elseif(channel_power_new ~=  channel_power)
            identical = 0;
        elseif(D_new ~= D)
            identical = 0;
        elseif(alphaDB_new ~= alphaDB)
            identical = 0;
        elseif(gam_new ~= gam)
            identical = 0;
        elseif(res_Disp_new ~= res_Disp)
            identical = 0;
        elseif(HalfWindow_new ~= HalfWindow)
            identical = 0;
        end
    end

    if(identical == 1)
        if(length(fs_new) == length(fs))
            for i=1:length(fs_new)
                if(fs_new(i) ~= fs(i))
                    identical = 0;
                end
            end
        end
    end
