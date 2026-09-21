import re
import os

def fix_returns_and_status(content):
    # 1. Rename TF_Status* status to TF_Status* out_status
    content = re.sub(r'TF_Status\*\s*status', r'TF_Status* out_status', content)
    content = re.sub(r'TF_Status\*\s*out_status\)', r'TF_Status* out_status)', content)
    
    # Remove static inline void init_.*
    content = re.sub(r'    static inline void init_[^{]+{[^}]+}\n', '', content, flags=re.MULTILINE|re.DOTALL)

    return content

# We will process each file manually or semi-manually to ensure correct out_ parameter conversions
