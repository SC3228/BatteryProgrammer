from ee.utils.bqstudio import BQStudioGG
from tkinter import Tk
from tkinter.filedialog import askopenfilename
from pandas import DataFrame, concat
from numpy import where

#Load File
Tk().withdraw()
filename = askopenfilename()
candidate = BQStudioGG.load_from_csv(filename)

reference = BQStudioGG(candidate.header, candidate.data_frame.copy())

'''
# Section 3 - CONFIGURATION
'''

# Section 3.1 - CONFIGURATION - Settings
reference.data_frame.loc['Settings','Configuration','Power Config'][' Parameter Value'] = 0x01
reference.data_frame.loc['Settings','Configuration','SOC Flag Config A'][' Parameter Value'] = 0xCAC   # 3.1.2
reference.data_frame.loc['Settings','Configuration','SOC Flag Config B'][' Parameter Value'] = 0xAC   # 3.1.3
reference.data_frame.loc['Settings','Configuration','IT Gauging Configuration'][' Parameter Value'] = 0x114     # 3.1.4
reference.data_frame.loc['Settings','Configuration','Balancing Configuration'][' Parameter Value'] = 0x01       # 3.1.5
reference.data_frame.loc['Settings','Configuration','FET Options'][' Parameter Value'] = 0x04          # 3.1.6
reference.data_frame.loc['Settings','Configuration','Sbs Gauging Configuration'][' Parameter Value'] = 0x00     # 3.1.7
reference.data_frame.loc['Settings','Configuration','Temperature Enable'][' Parameter Value'] = 0x02          # 3.1.8
#
# # Section 3.2 - CONFIGURATION - Protection
reference.data_frame.loc['Settings','Protection','Protection Configuration'][' Parameter Value'] = 0x02      # 3.2.1
#
# # Section 3.3 - CONFIGURATION - LED Support
reference.data_frame.loc['Settings','LED Support','Cycle Count Threshold'][' Parameter Value'] = 65535       # 3.3.1
reference.data_frame.loc['Settings','LED Support','Capacity Ratio'][' Parameter Value'] = 1             # 3.3.2
#
# # Section 3.4 - CONFIGURATION - Permanent Failure
reference.data_frame.loc['Settings','Permanent Failure','Enabled PF A'][' Parameter Value'] = 0x02         # 3.4.1
reference.data_frame.loc['Settings','Permanent Failure','Enabled PF C'][' Parameter Value'] = 0x03         # 3.4.2
#
# # Section 3.5 - CONFIGURATION - Manufacturing
reference.data_frame.loc['Settings','Manufacturing','Mfg Status init'][' Parameter Value'] = 0x38         # 3.5.1
#
# '''
# # Section 4 - ADVANCED CHARGING ALGORITHMS
# '''
#
# # Section 4.1 - ADVANCED CHARGING ALGORITHMS - Termination Config
reference.data_frame.loc['Advanced Charge Algorithm','Termination Config','Charge Term Taper Current'][' Parameter Value'] = 250   # 4.1.1
reference.data_frame.loc['Advanced Charge Algorithm','Termination Config','Charge Term Voltage'][' Parameter Value'] = 75          # 4.1.2
#
# '''
# # Section 5 - POWER
# '''
#
# # Section 5.1 - POWER - Shutdown Voltage
reference.data_frame.loc['Power','Shutdown','Shutdown Voltage'][' Parameter Value'] = 2300   # 5.1.1
reference.data_frame.loc['Power','Shutdown','Shutdown Time'][' Parameter Value'] = 30        # 5.1.2
#
# '''
# # Section 6 - GAS GAUGING
# '''
#
# # Section 6.1 - GAS GAUGING - Design
reference.data_frame.loc['Gas Gauging','Design','Design Capacity mAh'][' Parameter Value'] = 6800   # 6.1.1
reference.data_frame.loc['Gas Gauging','Design','Design Voltage'][' Parameter Value'] =  7270        # 6.1.2
#
# # Section 6.2 - GAS GAUGING - Cycle
reference.data_frame.loc['Gas Gauging','Cycle','Cycle Count Percentage'][' Parameter Value'] = 80   # 6.2.1
#
# # Section 6.3 - GAS GAUGING - FC
reference.data_frame.loc['Gas Gauging','FC','Clear Voltage Threshold'][' Parameter Value'] = 4100   # 6.3.1
reference.data_frame.loc['Gas Gauging','FC','Clear % RSOC Threshold'][' Parameter Value'] = 95      # 6.3.2
#
# # Section 6.4 - GAS GAUGING - TCA
reference.data_frame.loc['Gas Gauging','TC','Clear Voltage Threshold'][' Parameter Value'] = 4100      # 6.4.1
reference.data_frame.loc['Gas Gauging','TC','Clear % RSOC Threshold'][' Parameter Value'] = 95         # 6.4.2
#
# # Section 6.5 Omitted
#
# # Section 6.6 - GAS GAUGING - IT Cfg
reference.data_frame.loc['Gas Gauging','IT Cfg','Term Voltage'][' Parameter Value'] = 5000       # 6.6.1
reference.data_frame.loc['Gas Gauging','IT Cfg','Term Voltage Delta'][' Parameter Value'] = 800  # 6.6.2
reference.data_frame.loc['Gas Gauging','IT Cfg','Load Select'][' Parameter Value'] = 7           # 6.6.3
reference.data_frame.loc['Gas Gauging','IT Cfg','Load Mode'][' Parameter Value'] = 0             # 6.6.4
reference.data_frame.loc['Gas Gauging','IT Cfg','Reserve Cap-mAh'][' Parameter Value'] = 0       # 6.6.5
#
# # Section 6.7 - GAS GAUGING - Condition Flag
reference.data_frame.loc['Gas Gauging','Condition Flag','Max Error Limit'][' Parameter Value'] = 8   # 6.7.1
#
# # Section 6.8 - GAS GAUGING - SOH
reference.data_frame.loc['Gas Gauging','SoH','SoH Load Rate'][' Parameter Value'] = 25.5   # 6.8.1
#
# '''
# # Section 7 - SBS CONFIGURATION
# '''
#
# # Section 7.1 - SBS CONFIGURATION - Data
reference.data_frame.loc['SBS Configuration','Data','Manufacturer Name'][' Parameter Value'] = 'ZebraTechnologiesInc'   # 7.1.1
reference.data_frame.loc['SBS Configuration','Data','Device Name'][' Parameter Value'] = 'P1089760-002'           # 7.1.2

# Find differences
#https://stackoverflow.com/a/42652112/1440598
df=concat({'reference':reference.data_frame, 'candidate':candidate.data_frame})
df.drop_duplicates(keep=False, inplace=True)
print(df.to_string())

# Print differences
def test(val):
    name = val.name
    expeceted = reference.data_frame.loc[val.name][' Parameter Value']
    got = candidate.data_frame.loc[val.name][' Parameter Value']
    if type(expeceted) is int:
        print('name: {:>60} expected: {:>20}({:04x}) got: {:>20}({:04x})'.format(str(name), str(expeceted), expeceted, str(got), got))
    else:
        print('name: {:>60} expected: {:>26} got: {:>26}'.format(str(name), str(expeceted), str(got)))
df.loc['reference'].apply(test, axis='columns')

